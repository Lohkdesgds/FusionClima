#pragma once

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

enum class CommStatus {FAILED, READY_FRESH, READY_HASBUF};

class AsyncSerial {
    const size_t max_buffer_len;
    std::vector<char> buf;
    std::mutex buf_m;
    std::condition_variable buf_c;
    std::thread thr;
    HANDLE handler = nullptr;
    bool keep_running = false;
    DWORD err{};
    COMSTAT st{};

    BOOL _upd_stat();

    void _read_async();
public:
    AsyncSerial(const size_t);
    ~AsyncSerial();

    CommStatus try_connect_port(const size_t, const DWORD = CBR_115200);

    bool has_data(const uint64_t ms = 10000);
    bool valid() const;
    operator bool() const;

    std::string getline(const bool = true);
    std::string read(const size_t, const bool = true);
    size_t write(const std::string&);    

    void close();
};