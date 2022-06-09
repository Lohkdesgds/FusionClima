#include "mylora.h"

uint16_t __loracode = 0;
std::mutex __lorasnd;
bool __lorahost;
lora_fire __lorafire;

lora_fire::lora_fire()
{
    has_ptr = false;
}

bool lora_fire::post(lora_dgram&& np)
{
    if (has_ptr) return false;
    ptr = std::move(np);
    has_ptr = true;

    return true;
}

lora_dgram lora_fire::wait()
{
    while(!has_ptr) delay(250);
    return std::move(ptr);
}

void lora_fire::free()
{
    has_ptr = false;
}

void __lorahook(int siz)
{
    if (siz <= sizeof(__loracode)) {
        mprint("[LoRa] Invalid package received (size).\n");
        return;
    }

    uint16_t cod = 0;
    lora_dgram dg;

    //dg.rssi = LoRa.packetRssi();
    //dg.snr = LoRa.packetSnr();
    
    LoRa.readBytes((char*)&cod, sizeof(cod));
    siz -= sizeof(cod);

    if (cod != __loracode || siz == 0) {
        mprint("[LoRa] Invalid package received (code or null).\n");
        return;
    }
    
    dg.ptr = std::unique_ptr<char[]>(new char[siz]);
    dg.ptr_len = static_cast<uint32_t>(siz);
    
    LoRa.readBytes((char*)dg.ptr.get(), siz);
    siz = 0;

    mprint("[LoRa] Posting task!\n");
    if (!__lorafire.post(std::move(dg))) {
        mprint("[LoRa] Could not post task! Lost package.\n");
    }
}

bool lora_begin(bool host, long freq, uint16_t code)
{
    __loracode = code;
    __lorahost = host;
    if (!LoRa.begin(freq, true)) {
        mprint("[LoRa] Failed to start LoRa.\n");
        return false;
    }
    LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
    
    if (host) xTaskCreatePinnedToCore(__lora_recv_host, "LoRaRecvAsync", 3072, nullptr, tskIDLE_PRIORITY, nullptr, 1);
    else xTaskCreatePinnedToCore(__lora_recv_client, "LoRaRecvAsync", 3072, nullptr, tskIDLE_PRIORITY, nullptr, 1);

    LoRa.onReceive(__lorahook);
    LoRa.receive();
    if (host) xTaskCreatePinnedToCore(__lora_loop_host, "LoRaSendAuto", 3072, nullptr, 5, nullptr, 1);
}

bool __lora_send(char* buf, size_t len)
{
    std::lock_guard<std::mutex> l(__lorasnd);

    if (len == 0) {
        mprint("[LoRa] You tried to send a package with length 0.\n");
        return false;
    }

    if (!LoRa.beginPacket()) {
        mprint("[LoRa] Failed to begin packet.\n");
        LoRa.receive();
        return false;
    }

    LoRa.write((uint8_t*)&__loracode, sizeof(__loracode));
    LoRa.write((uint8_t*)buf, len);
    const bool gud = LoRa.endPacket();
    LoRa.receive();
    return gud;
}

void __lora_recv_client(void*)
{
    mprint("[LoRa] Async client recv is set and ready!\n");
    while(1) {
        lora_dgram a = __lorafire.wait();
        __lorafire.free();
        
        __rawshrdata.m_last_rssi = LoRa.packetRssi();

        mprint("[LoRa] Got one package of size %zu\n", a.ptr_len);

        if (a.ptr_len != sizeof(shared_data)) {
            mprint("[LoRa] Packet does not match size\n");
            continue;
        }

        const shared_data& it = *((shared_data*)a.ptr.get());
        __rawshrdata.import(it);
        __rawshrdata.m_last_send = TIME_NOW_MS;

    }
}

void __lora_recv_host(void*)
{
    mprint("[LoRa] Async host recv is set and ready!\n");
    while(1) {
        lora_dgram a = __lorafire.wait();
        __lorafire.free();

        __rawshrdata.m_last_rssi = LoRa.packetRssi();
        
        if (a.ptr_len != sizeof(request_model)) {
            mprint("[LoRa] Packet does not match size\n");
            continue;
        }

        const request_model& it = *((request_model*)a.ptr.get());

        switch(it.request_type) {
        case 1:
        {
            mprint("[LoRa] External device requested update.\n");
            shared_data cpy = __rawshrdata;
            if (__lora_send((char*)&cpy, sizeof(cpy))) {
                __rawshrdata.m_last_send = TIME_NOW_MS;
            }
            else {
                mprint("[LoRa] Failed to send packet\n");
            }
        }
            break;
        default:
        {
            mprint("[LoRa] Host got invalid request type.\n");
        }
            break;
        }
    }
}

void __lora_loop_host(void*)
{
    mprint("[LoRa] Autosend task started!\n");
    while(1) {
        delay(data_time_to_send_next);
        shared_data cpy = __rawshrdata;
        if (__lora_send((char*)&cpy, sizeof(cpy))) {
            __rawshrdata.m_last_send = TIME_NOW_MS;
        }
        else {
            mprint("[LoRa] Failed to send packet\n");
        }
    }
}