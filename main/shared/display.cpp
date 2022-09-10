#include "display.h"

constexpr unsigned int I2C_TIMEOUT_MS = 1000;
static const char* TAG = "DISPLAY_INTERNAL";

extern "C" {

spi_device_handle_t handle_spi;      // SPI handle.
i2c_cmd_handle_t    handle_i2c;      // I2C handle.
u8g2_esp32_hal_t    u8g2_esp32_hal;  // HAL state data.

#define U8G2_ESP32_HAL_UNDEFINED (-1)
#define I2C_MASTER_NUM I2C_NUM_1           //  I2C port number for master dev
#define I2C_MASTER_TX_BUF_DISABLE   0      //  I2C master do not need buffer
#define I2C_MASTER_RX_BUF_DISABLE   0      //  I2C master do not need buffer
#define I2C_MASTER_FREQ_HZ          800000  //  I2C master clock frequency
#define ACK_CHECK_EN   0x1                 //  I2C master will check ack from slave
#define ACK_CHECK_DIS  0x0                 //  I2C master will not check ack from slave

void U8G2_CLEANUP_DEFAULT(u8g2_esp32_hal_t* targ) {
    targ->clk = (gpio_num_t)U8G2_ESP32_HAL_UNDEFINED;
    targ->mosi = (gpio_num_t)U8G2_ESP32_HAL_UNDEFINED;
    targ->sda = (gpio_num_t)U8G2_ESP32_HAL_UNDEFINED;
    targ->scl = (gpio_num_t)U8G2_ESP32_HAL_UNDEFINED;
    targ->cs = (gpio_num_t)U8G2_ESP32_HAL_UNDEFINED;
    targ->reset = (gpio_num_t)U8G2_ESP32_HAL_UNDEFINED;
    targ->dc = (gpio_num_t)U8G2_ESP32_HAL_UNDEFINED;
}

/*
 * Initialze the ESP32 HAL.
 */
void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param) {
	u8g2_esp32_hal = u8g2_esp32_hal_param;
} // u8g2_esp32_hal_init

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle SPI communications.
 */
uint8_t u8g2_esp32_spi_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	ESP_LOGD(TAG, "spi_byte_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);
	switch(msg) {
		case U8X8_MSG_BYTE_SET_DC:
			if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.dc, arg_int);
			}
			break;

		case U8X8_MSG_BYTE_INIT: {
			if (u8g2_esp32_hal.clk == U8G2_ESP32_HAL_UNDEFINED ||
					u8g2_esp32_hal.mosi == U8G2_ESP32_HAL_UNDEFINED ||
					u8g2_esp32_hal.cs == U8G2_ESP32_HAL_UNDEFINED) {
				break;
			}

		  spi_bus_config_t bus_config;
                  memset(&bus_config, 0, sizeof(spi_bus_config_t));
		  bus_config.sclk_io_num   = u8g2_esp32_hal.clk; // CLK
		  bus_config.mosi_io_num   = u8g2_esp32_hal.mosi; // MOSI
		  bus_config.miso_io_num   = -1; // MISO
		  bus_config.quadwp_io_num = -1; // Not used
		  bus_config.quadhd_io_num = -1; // Not used
		  //ESP_LOGI(TAG, "... Initializing bus.");
		  ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1));

		  spi_device_interface_config_t dev_config;
		  dev_config.address_bits     = 0;
		  dev_config.command_bits     = 0;
		  dev_config.dummy_bits       = 0;
		  dev_config.mode             = 0;
		  dev_config.duty_cycle_pos   = 0;
		  dev_config.cs_ena_posttrans = 0;
		  dev_config.cs_ena_pretrans  = 0;
		  dev_config.clock_speed_hz   = 10000;
		  dev_config.spics_io_num     = u8g2_esp32_hal.cs;
		  dev_config.flags            = 0;
		  dev_config.queue_size       = 200;
		  dev_config.pre_cb           = NULL;
		  dev_config.post_cb          = NULL;
		  //ESP_LOGI(TAG, "... Adding device bus.");
		  ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &handle_spi));

		  break;
		}

		case U8X8_MSG_BYTE_SEND: {
			spi_transaction_t trans_desc;
			trans_desc.addr      = 0;
			trans_desc.cmd   	 = 0;
			trans_desc.flags     = 0;
			trans_desc.length    = 8 * arg_int; // Number of bits NOT number of bytes.
			trans_desc.rxlength  = 0;
			trans_desc.tx_buffer = arg_ptr;
			trans_desc.rx_buffer = NULL;

			//ESP_LOGI(TAG, "... Transmitting %d bytes.", arg_int);
			ESP_ERROR_CHECK(spi_device_transmit(handle_spi, &trans_desc));
			break;
		}
	}
	return 0;
} // u8g2_esp32_spi_byte_cb

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle I2C communications.
 */
uint8_t u8g2_esp32_i2c_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	ESP_LOGD(TAG, "i2c_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);

	switch(msg) {
		case U8X8_MSG_BYTE_SET_DC: {
			if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.dc, arg_int);
			}
			break;
		}

		case U8X8_MSG_BYTE_INIT: {
			if (u8g2_esp32_hal.sda == U8G2_ESP32_HAL_UNDEFINED ||
					u8g2_esp32_hal.scl == U8G2_ESP32_HAL_UNDEFINED) {
				break;
			}

		    i2c_config_t conf;
		    memset(&conf, 0, sizeof(i2c_config_t));
		    conf.mode = I2C_MODE_MASTER;
			ESP_LOGI(TAG, "sda_io_num %d", u8g2_esp32_hal.sda);
		    conf.sda_io_num = u8g2_esp32_hal.sda;
		    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
			ESP_LOGI(TAG, "scl_io_num %d", u8g2_esp32_hal.scl);
		    conf.scl_io_num = u8g2_esp32_hal.scl;
		    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
			ESP_LOGI(TAG, "clk_speed %d", I2C_MASTER_FREQ_HZ);
		    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
			ESP_LOGI(TAG, "i2c_param_config %d", conf.mode);
		    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
			ESP_LOGI(TAG, "i2c_driver_install %d", I2C_MASTER_NUM);
		    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
			break;
		}

		case U8X8_MSG_BYTE_SEND: {
			uint8_t* data_ptr = (uint8_t*)arg_ptr;
			ESP_LOG_BUFFER_HEXDUMP(TAG, data_ptr, arg_int, ESP_LOG_VERBOSE);

			while( arg_int > 0 ) {
			   ESP_ERROR_CHECK(i2c_master_write_byte(handle_i2c, *data_ptr, ACK_CHECK_EN));
			   data_ptr++;
			   arg_int--;
			}
			break;
		}

		case U8X8_MSG_BYTE_START_TRANSFER: {
			uint8_t i2c_address = u8x8_GetI2CAddress(u8x8);
			handle_i2c = i2c_cmd_link_create();
			ESP_LOGD(TAG, "Start I2C transfer to %02X.", i2c_address>>1);
			ESP_ERROR_CHECK(i2c_master_start(handle_i2c));
			ESP_ERROR_CHECK(i2c_master_write_byte(handle_i2c, i2c_address | I2C_MASTER_WRITE, ACK_CHECK_EN));
			break;
		}

		case U8X8_MSG_BYTE_END_TRANSFER: {
			ESP_LOGD(TAG, "End I2C transfer.");
			ESP_ERROR_CHECK(i2c_master_stop(handle_i2c));
			ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, handle_i2c, I2C_TIMEOUT_MS / portTICK_RATE_MS));
			i2c_cmd_link_delete(handle_i2c);
			break;
		}
	}
	return 0;
} // u8g2_esp32_i2c_byte_cb

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle callbacks for GPIO and delay functions.
 */
uint8_t u8g2_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	ESP_LOGD(TAG, "gpio_and_delay_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);

	switch(msg) {
	// Initialize the GPIO and DELAY HAL functions.  If the pins for DC and RESET have been
	// specified then we define those pins as GPIO outputs.
		case U8X8_MSG_GPIO_AND_DELAY_INIT: {
			uint64_t bitmask = 0;
			if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1ull<<u8g2_esp32_hal.dc);
			}
			if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1ull<<u8g2_esp32_hal.reset);
			}
			if (u8g2_esp32_hal.cs != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1ull<<u8g2_esp32_hal.cs);
			}

            if (bitmask==0) {
            	break;
            }
			gpio_config_t gpioConfig;
			gpioConfig.pin_bit_mask = bitmask;
			gpioConfig.mode         = GPIO_MODE_OUTPUT;
			gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
			gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
			gpioConfig.intr_type    = GPIO_INTR_DISABLE;
			gpio_config(&gpioConfig);
			break;
		}

	// Set the GPIO reset pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_RESET:
			if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.reset, arg_int);
			}
			break;
	// Set the GPIO client select pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_CS:
			if (u8g2_esp32_hal.cs != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.cs, arg_int);
			}
			break;
	// Set the Software I²C pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_I2C_CLOCK:
			if (u8g2_esp32_hal.scl != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.scl, arg_int);
//				printf("%c",(arg_int==1?'C':'c'));
			}
			break;
	// Set the Software I²C pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_I2C_DATA:
			if (u8g2_esp32_hal.sda != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.sda, arg_int);
//				printf("%c",(arg_int==1?'D':'d'));
			}
			break;

	// Delay for the number of milliseconds passed in through arg_int.
		case U8X8_MSG_DELAY_MILLI:
			vTaskDelay(arg_int/portTICK_PERIOD_MS);
			break;
	}
	return 0;
} // u8g2_esp32_gpio_and_delay_cb

}

void Display::setup(const gpio_num_t sda, const gpio_num_t scl, const gpio_num_t rst)
{
	u8g2_esp32_hal_t u8g2_esp32_hal;
    U8G2_CLEANUP_DEFAULT(&u8g2_esp32_hal);
	u8g2_esp32_hal.sda   = sda;
	u8g2_esp32_hal.scl   = scl;
	u8g2_esp32_hal.reset = rst;
	u8g2_esp32_hal_init(u8g2_esp32_hal);
    
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(
		&m_dsp,
		U8G2_R0,
		u8g2_esp32_i2c_byte_cb,
		u8g2_esp32_gpio_and_delay_cb);
	u8x8_SetI2CAddress(&m_dsp.u8x8, 0x78);
	u8g2_InitDisplay(&m_dsp);
	u8g2_SetPowerSave(&m_dsp, 0);
}

void Display::SendBuffer()
{
    u8g2_SendBuffer(&m_dsp);
}
void Display::ClearBuffer()
{
    u8g2_ClearBuffer(&m_dsp);
}
void Display::FirstPage()
{
    u8g2_FirstPage(&m_dsp);
}
uint8_t Display::NextPage()
{
    return u8g2_NextPage(&m_dsp);
}
void Display::UpdateDisplayArea(uint8_t  tx, uint8_t ty, uint8_t tw, uint8_t th)
{
    u8g2_UpdateDisplayArea(&m_dsp, tx, ty, tw, th);
}
void Display::UpdateDisplay()
{
    u8g2_UpdateDisplay(&m_dsp);
}
void Display::DrawHVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
{
    u8g2_DrawHVLine(&m_dsp, x, y, len, dir);
}
void Display::DrawHLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len)
{
    u8g2_DrawHLine(&m_dsp, x, y, len);
}
void Display::DrawVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len)
{
    u8g2_DrawVLine(&m_dsp, x, y, len);
}
void Display::DrawPixel(u8g2_uint_t x, u8g2_uint_t y)
{
    u8g2_DrawPixel(&m_dsp, x, y);
}
void Display::SetDrawColor(uint8_t color)
{
    u8g2_SetDrawColor(&m_dsp, color);
}
void Display::SetBitmapMode(uint8_t is_transparent)
{
    u8g2_SetBitmapMode(&m_dsp, is_transparent);
}
void Display::DrawHorizontalBitmap(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, const uint8_t *b)
{
    u8g2_DrawHorizontalBitmap(&m_dsp, x, y, len,b);
}
void Display::DrawBitmap(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t cnt, u8g2_uint_t h, const uint8_t *bitmap)
{
    u8g2_DrawBitmap(&m_dsp, x, y, cnt, h,bitmap);
}
void Display::DrawXBM(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap)
{
    u8g2_DrawXBM(&m_dsp, x, y, w, h,bitmap);
}
void Display::DrawXBMP(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap)
{
    u8g2_DrawXBMP(&m_dsp, x, y, w, h,bitmap);
}
void Display::DrawCircle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option)
{
    u8g2_DrawCircle(&m_dsp, x0, y0, rad, option);
}
void Display::DrawDisc(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option)
{
    u8g2_DrawDisc(&m_dsp, x0, y0, rad, option);
}
void Display::DrawEllipse(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option)
{
    u8g2_DrawEllipse(&m_dsp, x0, y0, rx, ry, option);
}
void Display::DrawFilledEllipse(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option)
{
    u8g2_DrawFilledEllipse(&m_dsp, x0, y0, rx, ry, option);
}
void Display::DrawLine(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2)
{
    u8g2_DrawLine(&m_dsp, x1, y1, x2, y2);
}
void Display::DrawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
    u8g2_DrawBox(&m_dsp, x, y, w, h);
}
void Display::DrawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
    u8g2_DrawFrame(&m_dsp, x, y, w, h);
}
void Display::DrawRBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r)
{
    u8g2_DrawRBox(&m_dsp, x, y, w, h, r);
}
void Display::DrawRFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r)
{
    u8g2_DrawRFrame(&m_dsp, x, y, w, h, r);
}
void Display::DrawButtonFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t flags, u8g2_uint_t text_width, u8g2_uint_t padding_h, u8g2_uint_t padding_v)
{
    u8g2_DrawButtonFrame(&m_dsp, x, y, flags, text_width, padding_h, padding_v);
}
void Display::DrawButtonUTF8(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t flags, u8g2_uint_t width, u8g2_uint_t padding_h, u8g2_uint_t padding_v, const char *text)
{
    u8g2_DrawButtonUTF8(&m_dsp, x, y, flags, width, padding_h, padding_v, text);
}
void Display::ClearPolygonXY(void)
{
    u8g2_ClearPolygonXY();
}
void Display::AddPolygonXY(int16_t x, int16_t y)
{
    u8g2_AddPolygonXY(&m_dsp, x, y);
}
void Display::DrawPolygon()
{
    u8g2_DrawPolygon(&m_dsp);
}
void Display::DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    u8g2_DrawTriangle(&m_dsp, x0, y0, x1, y1, x2, y2);
}
uint8_t Display::GetKerning(u8g2_kerning_t *kerning, uint16_t e1, uint16_t e2)
{
    return u8g2_GetKerning(&m_dsp, kerning, e1, e2);
}
uint8_t Display::GetKerningByTable(const uint16_t *kt, uint16_t e1, uint16_t e2)
{
    return u8g2_GetKerningByTable(&m_dsp, kt, e1, e2);
}
void Display::SetFont(const uint8_t *font)
{
    u8g2_SetFont(&m_dsp, font);
}
void Display::SetFontMode(uint8_t is_transparent)
{
    u8g2_SetFontMode(&m_dsp, is_transparent);
}
uint8_t Display::IsGlyph(uint16_t requested_encoding)
{
    return u8g2_IsGlyph(&m_dsp, requested_encoding);
}
int8_t Display::GetGlyphWidth(uint16_t requested_encoding)
{
    return u8g2_GetGlyphWidth(&m_dsp, requested_encoding);
}
u8g2_uint_t Display::DrawGlyph(u8g2_uint_t x, u8g2_uint_t y, uint16_t encoding)
{
    return u8g2_DrawGlyph(&m_dsp, x, y, encoding);
}
u8g2_uint_t Display::DrawGlyphX2(u8g2_uint_t x, u8g2_uint_t y, uint16_t encoding)
{
    return u8g2_DrawGlyphX2(&m_dsp, x, y, encoding);
}
int8_t Display::GetStrX(const char *s)
{
    return u8g2_GetStrX(&m_dsp, s);	
}
void Display::SetFontDirection(uint8_t dir)
{
    u8g2_SetFontDirection(&m_dsp, dir);
}
u8g2_uint_t Display::DrawStr(u8g2_uint_t x, u8g2_uint_t y, const char *str)
{
    return u8g2_DrawStr(&m_dsp, x, y, str);
}
u8g2_uint_t Display::DrawStrX2(u8g2_uint_t x, u8g2_uint_t y, const char *str)
{
    return u8g2_DrawStrX2(&m_dsp, x, y, str);
}
u8g2_uint_t Display::DrawUTF8(u8g2_uint_t x, u8g2_uint_t y, const char *str)
{
    return u8g2_DrawUTF8(&m_dsp, x, y, str);
}
u8g2_uint_t Display::DrawUTF8X2(u8g2_uint_t x, u8g2_uint_t y, const char *str)
{
    return u8g2_DrawUTF8X2(&m_dsp, x, y, str);
}
u8g2_uint_t Display::DrawExtendedUTF8(u8g2_uint_t x, u8g2_uint_t y, uint8_t to_left, u8g2_kerning_t *kerning, const char *str)
{
    return u8g2_DrawExtendedUTF8(&m_dsp, x, y, to_left,kerning, str);
}
u8g2_uint_t Display::DrawExtUTF8(u8g2_uint_t x, u8g2_uint_t y, uint8_t to_left, const uint16_t *kerning_table, const char *str)
{
    return u8g2_DrawExtUTF8(&m_dsp, x, y, to_left,kerning_table, str);
}
uint8_t Display::IsAllValidUTF8(const char *str)
{
    return u8g2_IsAllValidUTF8(&m_dsp, str);
}
u8g2_uint_t Display::GetStrWidth(const char *s)
{
    return u8g2_GetStrWidth(&m_dsp, s);
}
u8g2_uint_t Display::GetUTF8Width(const char *str)
{
    return u8g2_GetUTF8Width(&m_dsp, str);
}
void Display::SetFontPosBaseline()
{
    u8g2_SetFontPosBaseline(&m_dsp);
}
void Display::SetFontPosBottom()
{
    u8g2_SetFontPosBottom(&m_dsp);
}
void Display::SetFontPosTop()
{
    u8g2_SetFontPosTop(&m_dsp);
}
void Display::SetFontPosCenter()
{
    u8g2_SetFontPosCenter(&m_dsp);
}
void Display::SetFontRefHeightText()
{
    u8g2_SetFontRefHeightText(&m_dsp);
}
void Display::SetFontRefHeightExtendedText()
{
    u8g2_SetFontRefHeightExtendedText(&m_dsp);
}
void Display::SetFontRefHeightAll()
{
    u8g2_SetFontRefHeightAll(&m_dsp);
}
void Display::DrawUTF8Line(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, const char *s, uint8_t border_size, uint8_t is_invert)
{
    u8g2_DrawUTF8Line(&m_dsp, x, y, w, s, border_size, is_invert);
}
u8g2_uint_t Display::DrawUTF8Lines(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t line_height, const char *s)
{
    return u8g2_DrawUTF8Lines(&m_dsp, x, y, w, line_height, s);
}
uint8_t Display::UserInterfaceSelectionList(const char *title, uint8_t start_pos, const char *sl)
{
    return u8g2_UserInterfaceSelectionList(&m_dsp, title, start_pos, sl);
}