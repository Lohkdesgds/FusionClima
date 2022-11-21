#pragma once

#include "protocol_combo.h"
#define ESP_PLATFORM
#include <U8g2lib.h>
#include "heltec.h"
#include <string.h>
#include "protocol_combo.h"
#include "references.h"
#include <mutex>

struct AsDongle {
    QueueHandle_t dongle_que = xQueueCreate(4, sizeof(Comm::generic_format));
    volatile float last_temp = 0.0f;
    volatile float last_hum = 0.0f;
    uint64_t last_mac = 0;
    volatile float smooth_rssi = 0.0f;
    volatile float smooth_snr = 0.0f;
    volatile float real_rssi = 0.0f;
    volatile float real_snr = 0.0f;
    volatile uint8_t packet_counter = 0;
    Comm::usb_format_raw dgl_ev{};
    std::mutex dgl_mtx;
};

// ask for information nearby
void dongle_tweet();