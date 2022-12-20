#pragma once

#include "protocol_combo.h"
#define ESP_PLATFORM
#include <stdint.h>
#include <DHT.h>
#include <DHT_U.h>
#include <U8g2lib.h>
#include "heltec.h"
#include <string>
#include "references.h"

constexpr int pin_DHT22 = 32;
constexpr uint32_t mnt_loop_time_ds = 600; // 600 ds (deci-sec), broadcast
constexpr uint32_t mnt_update_time_ds = 50; // 50 ds, read stuff

struct AsMonitor {
    DHT dht;
    bool should_tweet_soon = true;
    uint64_t last_tweeter = 0;
    uint64_t tweet_count = 0;
    
    AsMonitor();
};
