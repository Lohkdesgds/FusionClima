#include "buttons.h"

gpio_event_mapping _fmap;
extern const int adc_channel_io_map[SOC_ADC_PERIPH_NUM][SOC_ADC_MAX_CHANNEL_NUM];

#define ADC_GET_IO_NUM(periph, channel) (adc_channel_io_map[periph][channel])

adc_channel_t custom_to_adc(gpio_num_t t, ADC_num& aa)
{
    const auto adcn_pad_get_io_num_nothrow = [](adc_channel_t channel, gpio_num_t *gpio_num, bool adc2questionmark){
        if (!(channel < SOC_ADC_CHANNEL_NUM(adc2questionmark ? 1 : 0))) return false;
        int io = ADC_GET_IO_NUM(adc2questionmark ? 1 : 0, channel);
        if (io < 0) return false;        
        *gpio_num = (gpio_num_t)io;
        return true;
    };
    const auto adcn = [&](adc_channel_t channel, gpio_num_t *gpio_num) {
        if (adcn_pad_get_io_num_nothrow(channel, gpio_num, false)) return 1;
        if (adcn_pad_get_io_num_nothrow(channel, gpio_num, true)) return 2;
        return 0;
    };
    
    for(uint32_t p = ADC_CHANNEL_0; p < ADC_CHANNEL_MAX; ++p) {
        gpio_num_t cp{};
        switch(adcn(static_cast<adc_channel_t>(p), &cp)) {
        case 1: // ADC1
            if (cp != t) break;
            aa = ADC_num::ADC1;
            return static_cast<adc_channel_t>(cp);
        case 2: // ADC2
            if (cp != t) break;
            aa = ADC_num::ADC2;
            return static_cast<adc_channel_t>(cp);
        default:
            break;
        }
    }
    return ADC_CHANNEL_MAX;
}

dac_channel_t custom_to_dac(gpio_num_t p)
{
    switch(p) {
#ifdef CONFIG_IDF_TARGET_ESP32S2
    case GPIO_NUM_17:
#else // ESP32
    case GPIO_NUM_25:
#endif
        return DAC_CHANNEL_1;
#ifdef CONFIG_IDF_TARGET_ESP32S2
    case GPIO_NUM_18:
#else // ESP32
    case GPIO_NUM_26:
#endif
        return DAC_CHANNEL_2;
    default:
        return DAC_CHANNEL_MAX;
    }
}

gpio_config_custom& gpio_config_custom::set_mode(gpio_mode_t a)
{
    c.mode = a;
    return *this;
}

gpio_config_custom& gpio_config_custom::set_pull_up(bool a)
{
    c.pull_up_en = a ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    return *this;
}

gpio_config_custom& gpio_config_custom::set_pull_down(bool a)
{
    c.pull_down_en = a ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    return *this;
}

gpio_config_custom& gpio_config_custom::set_trigger(gpio_int_type_t a)
{
    c.intr_type = a;
    return *this;
}

gpio_config_custom& gpio_config_custom::set_pin(gpio_num_t a)
{
    c.pin_bit_mask = BIT(static_cast<size_t>(a));
    return *this;
}

bool custom_enable_gpio_functional(bool en, BaseType_t core)
{
    if (en && !_fmap.thr) {
        if (gpio_isr_register(__custom_gpio_handler, nullptr, 0, &_fmap.gpio_reg) != ESP_OK) return false;
        if (xTaskCreatePinnedToCore(__async_gpio_handler, "asyncbtn", 2048, nullptr, 4, &_fmap.thr, core) != ESP_OK) return false;
    }
    else if (!en && _fmap.thr) {
        vTaskDelete(_fmap.thr);
        _fmap.thr = nullptr;
        gpio_uninstall_isr_service();
    }
    return true;
}

bool custom_gpio_setup(gpio_config_custom conf)
{
    return gpio_config(&conf.c) == ESP_OK;
}

void custom_gpio_map_to_function(gpio_num_t id, std::function<void(bool)> f)
{
    std::lock_guard<std::mutex> l(_fmap.fmap_m);
    _fmap.fmap[static_cast<size_t>(id)] = f;
}

bool custom_gpio_setup_analogread(std::initializer_list<gpio_num_t> ids, adc_atten_t atten, adc_bits_width_t wd)
{
    static bool has_adc1_setup = false;

    for(const auto& i : ids) {
        ADC_num typ;
        adc_channel_t ch = custom_to_adc(i, typ);
        if (ch == ADC_CHANNEL_MAX) return false; // one invalid == bad

        switch(typ) {
        case ADC_num::ADC1:
        {
            if (!has_adc1_setup) 
                adc1_config_width(wd); // this has to be called before any readings

            has_adc1_setup = true;
            // this has to be called before reading the channel.
            adc1_config_channel_atten(static_cast<adc1_channel_t>(ch), atten);
        }
        break;
        case ADC_num::ADC2:
        {
            // this has to be called before reading the channel.
            adc2_config_channel_atten(static_cast<adc2_channel_t>(ch), atten);
        }
        break;
        }
    }

    return true;
}

bool custom_digital_read_cache(gpio_num_t id)
{
    if (id == GPIO_NUM_MAX) return false;
    return _fmap.last_state[static_cast<size_t>(id)];
}

bool custom_digital_read(gpio_num_t id)
{
    return gpio_get_level(id);
}

bool custom_digital_write(gpio_num_t id, bool b)
{
    return gpio_set_level(id, b) == ESP_OK;
}

int32_t custom_analog_read(gpio_num_t p, adc_bits_width_t w)
{
    int i;
    ADC_num typ;

    adc_channel_t ch = custom_to_adc(p, typ);
    if (ch == ADC_CHANNEL_MAX) return -1; // one invalid == bad

    switch(typ) {
    case ADC_num::ADC1:
        return adc1_get_raw(static_cast<adc1_channel_t>(p));
    case ADC_num::ADC2:
        if (adc2_get_raw(static_cast<adc2_channel_t>(p), w, &i) != ESP_OK) return -1;
        break;
    }

    return i;
}

int32_t custom_analog_read(gpio_num_t p)
{
    return custom_analog_read(p, static_cast<adc_bits_width_t>(SOC_ADC_MAX_BITWIDTH));
}

bool custom_analog_write(gpio_num_t p, float perc)
{
    dac_channel_t ch = custom_to_dac(p);
    if (ch == DAC_CHANNEL_MAX) return false;
    if (perc == 0.0f) {
        if (dac_output_disable(ch) != ESP_OK) return false;
        return true;
    }
    else {
        if (dac_output_enable(ch) != ESP_OK) return false;
        if (dac_output_voltage(ch, perc * 255) != ESP_OK) return false;
    }
    return true;
}

void __custom_gpio_handler(void* unnused)
{
    if (!_fmap.thr) return; // stop if no thread

    uint32_t r_0 = READ_PERI_REG(GPIO_STATUS_REG); // [0..31]
    uint32_t r_1 = READ_PERI_REG(GPIO_STATUS1_REG);// [32..39]

    // clr
    SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, r_0);  
    SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, r_1);

    for(size_t id = 0; id < GPIO_NUM_MAX; ++id) {
        if (id < 32) {
            if (r_0 & BIT(id)) { // ID is triggered
                gpio_event_mapping::_ev ev;
                ev.id = static_cast<gpio_num_t>(id);
                _fmap.last_state[id] = ev.rise = gpio_get_level(static_cast<gpio_num_t>(id));
                if (xQueueSendToBackFromISR(_fmap.que, &ev, NULL) == errQUEUE_FULL)
                    break; // stop thinking too much
            }
        }
    }
}

void __async_gpio_handler(void* unnused)
{
    while(1) {
        gpio_event_mapping::_ev ev;
        if (xQueueReceive(_fmap.que, &ev, 0) == pdFALSE){
            delay(20);
            continue;
        }

        std::lock_guard<std::mutex> l(_fmap.fmap_m);
        const auto& f = _fmap.fmap[static_cast<size_t>(ev.id)];
        if (f) f(ev.rise);
    }
    vTaskDelete(NULL);
}



//
//__Dial __g_dial{ button_pin_press, button_pin_rot_r, button_pin_rot_l };
//
//__DialInternalEvent::__DialInternalEvent(dial_button_kind k, gpio_num_t n)
//    : index(k), pin(n)
//{    
//}
//
//__Dial::__Dial(const gpio_num_t p, const gpio_num_t r, const gpio_num_t l)
//    : m_reg({
//        __DialInternalEvent{dial_button_kind::KBUTTON, p},
//        __DialInternalEvent{dial_button_kind::KRIGHT, r},
//        __DialInternalEvent{dial_button_kind::KLEFT, l}
//    })
//{
//}
//
//void __Dial::__enqueue(dialevent ev)
//{
//    xQueueSendToBackFromISR(queue, &ev, NULL);
//}
//
//void __Dial::__post(dialevent ev)
//{
//    if (m_func) m_func(ev);
//}
//
//__DialInternalEvent& __Dial::__get_iev(dial_button_kind e)
//{
//    return m_reg[static_cast<size_t>(e)];
//}
//
//void __Dial::setup()
//{    
//    queue = xQueueCreate(10, sizeof(dialevent));
//    xTaskCreate(__handle_g_dial_async, "dialasync", 4096, queue, 4, NULL);
//
//	gpio_config_t gpioConfig;
//	gpioConfig.mode         = GPIO_MODE_INPUT;
//	gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
//	gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
//	gpioConfig.intr_type    = GPIO_INTR_ANYEDGE;
//    
//	for(size_t p = 0; p < m_reg.size(); ++p) {
//        gpioConfig.pin_bit_mask = BIT(m_reg[p].pin);
//	    gpio_config(&gpioConfig);
//    }
//
//	gpio_install_isr_service(0);
//
//	for(size_t p = 0; p < m_reg.size(); ++p) {
//	    gpio_isr_handler_add(m_reg[p].pin, __handle_g_dial, (void*)&m_reg[p]);
//    }
//}
//
//void __Dial::on_event(std::function<void(dialevent)> f)
//{
//    m_func = f;
//}
//
//void IRAM_ATTR __handle_g_dial(void* dev)
//{
//    if (!dev) return;
//    __DialInternalEvent& ev = *(__DialInternalEvent*)dev;
//    const bool lvl = gpio_get_level(ev.pin);
//
//    const auto _t = millis();
//
//    if (ev.hold == lvl) { // no change, update time and exit
//        ev.last = _t;
//        return;
//    }
//    // had change.
//    
//    dialevent nev;
//    nev.since_last = _t - ev.last; // diff time
//
//    // update last ev
//    ev.last = _t;
//    ev.hold = lvl;
//
//    switch(ev.index) {
//    case dial_button_kind::KBUTTON:
//        nev.ev = lvl ? dial_trigger_type::BUTTON_CLICK : dial_trigger_type::BUTTON_RELEASE;
//        break;
//    case dial_button_kind::KRIGHT:
//        //if (nev.since_last > trigger_threshold) { // first case of this
//        //    __DialInternalEvent& oth = __g_dial.__get_iev(dial_button_kind::KLEFT);
//        //    if (_t - oth.last > trigger_threshold) return; // had not happened on the other, this is first case
//        //    // it's this first time and the other has had an event last millisec
//        //    nev.ev = dial_trigger_type::MOV_CLOCKWISE;
//        //}
//        //else return; // just save, no ev
//        ev._hasread = false;
//        return;
//    case dial_button_kind::KLEFT:
//        //if (nev.since_last > trigger_threshold) { // first case of this
//        //    __DialInternalEvent& oth = __g_dial.__get_iev(dial_button_kind::KRIGHT);
//        //    if (_t - oth.last > trigger_threshold) return; // had not happened on the other, this is first case
//        //    // it's this first time and the other has had an event last millisec
//        //    nev.ev = dial_trigger_type::MOV_COUNTERCLOCKWISE;
//        //}
//        //else return; // just save, no ev
//        ev._hasread = false;
//        return;
//    }
//    
//    __g_dial.__enqueue(nev);
//}
//
//void __handle_g_dial_async(void* queue)
//{
//    QueueHandle_t handl = (QueueHandle_t)queue;
//    __DialInternalEvent& rr = __g_dial.__get_iev(dial_button_kind::KRIGHT);
//    __DialInternalEvent& ll = __g_dial.__get_iev(dial_button_kind::KLEFT);
//
//    while (handl) {
//        dialevent ev;
//        if (xQueueReceive(handl, &ev, 0) == pdFALSE){
//            
//            if (!rr._hasread && !ll._hasread) { // has rotation event ready
//                const auto _t = millis();
//                if (_t - rr.last <= trigger_threshold || _t - ll.last <= trigger_threshold) continue; // not ready
//
//                const bool Rfirst = rr.last < ll.last;
//                rr._hasread = ll._hasread = true;
//
//                ev.ev = Rfirst ? dial_trigger_type::MOV_COUNTERCLOCKWISE : dial_trigger_type::MOV_CLOCKWISE;
//                ev.since_last = 0;
//
//                __g_dial.__post(ev);
//                continue;
//            }
//            else delay(25);
//            continue;
//        }
//        // filter post processing
//        __g_dial.__post(ev);
//    }
//}
//
//void dial_start()
//{
//    static bool avoid = false;
//    if (!avoid) {
//        avoid = true;
//        __g_dial.setup();
//    }
//}
//
//void dial_set_event_function(std::function<void(dialevent)> f)
//{
//    __g_dial.on_event(f);
//}