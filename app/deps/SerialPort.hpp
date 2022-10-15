/*
* Author: Manash Kumar Mandal
* Modified Library introduced in Arduino Playground which does not work
* This works perfectly
* LICENSE: MIT
*/

#pragma once

//#define ARDUINO_WAIT_TIME 2000
//#define MAX_DATA_LENGTH 255

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

//class SerialPort
//{
//private:
//    HANDLE handler = nullptr;
//    COMSTAT status{};
//    DWORD errors{};
//    std::vector<std::string> errorsvec;
//
//    int readSerialPort(const char* buffer, unsigned int buf_size);
//    bool writeSerialPort(const char* buffer, unsigned int buf_size);
//public:
//    explicit SerialPort(const std::string& portName, const DWORD baudrate = CBR_115200);
//    ~SerialPort();
//
//    std::string read(const size_t lim, const bool hold = true);
//    bool write(const char*, const size_t);
//    bool write(const std::string&);
//
//    size_t buffered_size();
//
//    bool isConnected();
//    void closeSerial();
//
//    std::vector<std::string>& getErrorsList();
//};

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