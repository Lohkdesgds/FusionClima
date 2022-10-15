/*
* Author: Manash Kumar Mandal
* Modified Library introduced in Arduino Playground which does not work
* This works perfectly
* LICENSE: MIT
*/

#include "SerialPort.hpp"


//// Reading bytes from serial port to buffer;
//// returns read bytes count, or if error occurs, returns 0
//int SerialPort::readSerialPort(const char* buffer, unsigned int buf_size)
//{
//    DWORD bytesRead{};
//    unsigned int toRead = 0;
//
//    ClearCommError(handler, &(errors), &(status));
//
//    if (status.cbInQue > 0)
//    {
//        if (status.cbInQue > buf_size)
//        {
//            toRead = buf_size;
//        }
//        else
//        {
//            toRead = status.cbInQue;
//        }
//    }
//
//    memset((void*)buffer, 0, buf_size);
//
//    if (ReadFile(handler, (void*)buffer, toRead, &bytesRead, NULL))
//    {
//        return bytesRead;
//    }
//
//    return 0;
//}
//
//// Sending provided buffer to serial port;
//// returns true if succeed, false if not
//bool SerialPort::writeSerialPort(const char* buffer, unsigned int buf_size)
//{
//    DWORD bytesSend;
//
//    if (!WriteFile(handler, (void*)buffer, buf_size, &bytesSend, 0))
//    {
//        ClearCommError(handler, &errors, &status);
//        return false;
//    }
//
//    return true;
//}
//
//SerialPort::SerialPort(const std::string& portName, const DWORD baudrate)
//{
//    if (handler) CloseHandle(handler);
//    handler = CreateFileA(static_cast<LPCSTR>(portName.c_str()),
//                                GENERIC_READ | GENERIC_WRITE,
//                                0,
//                                NULL,
//                                OPEN_EXISTING,
//                                FILE_ATTRIBUTE_NORMAL,
//                                NULL);
//    if (handler == INVALID_HANDLE_VALUE)
//    {
//        if (GetLastError() == ERROR_FILE_NOT_FOUND)
//        {
//            errorsvec.push_back("ERROR: Handle was not attached.Reason : " + std::string(portName) + " not available");
//        }
//        else
//        {
//            errorsvec.push_back("ERROR: INVALID HANDLE");
//        }
//    }
//    else
//    {
//        DCB dcbSerialParameters = {0};
//
//        if (!GetCommState(handler, &dcbSerialParameters))
//        {
//            errorsvec.push_back("Failed to get current serial parameters");
//        }
//        else
//        {
//            dcbSerialParameters.BaudRate = baudrate;
//            dcbSerialParameters.ByteSize = 8;
//            dcbSerialParameters.StopBits = ONESTOPBIT;
//            dcbSerialParameters.Parity = NOPARITY;
//            dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;
//
//            if (!SetCommState(handler, &dcbSerialParameters))
//            {
//                errorsvec.push_back("ALERT: could not set serial port parameters");
//            }
//            else
//            {
//                PurgeComm(handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
//                Sleep(ARDUINO_WAIT_TIME);
//            }
//        }
//    }
//}
//
//SerialPort::~SerialPort()
//{
//    if (handler)
//    {
//        CloseHandle(handler);
//        handler = nullptr;
//    }
//}
//
//std::string SerialPort::read(const size_t lim, const bool hold)
//{
//    while (hold && buffered_size() == 0) std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    std::string str;
//    str.resize(lim);
//    int res = readSerialPort(str.data(), str.length());
//    if (res > 0) {
//        str.resize(static_cast<size_t>(res));
//        return str;
//    }
//    return {};
//}
//
//bool SerialPort::write(const char* buf, const size_t len)
//{
//    if (!len || !buf) return false;
//    return writeSerialPort(buf, static_cast<unsigned>(len));
//}
//
//bool SerialPort::write(const std::string& str)
//{
//    return write(str.data(), str.size());
//}
//
//size_t SerialPort::buffered_size()
//{
//    ClearCommError(handler, &(errors), &(status));
//
//    return static_cast<size_t>(status.cbInQue);
//}
//
//// Checking if serial port is connected
//bool SerialPort::isConnected()
//{
//    if (!ClearCommError(handler, &errors, &status))
//    {
//        if (handler) CloseHandle(handler);
//        handler = nullptr;
//    }
//
//    return handler;
//}
//
//void SerialPort::closeSerial()
//{
//    CloseHandle(handler);
//}
//
//std::vector<std::string>& SerialPort::getErrorsList()
//{
//    return errorsvec;
//}


BOOL AsyncSerial::_upd_stat()
{
    return ClearCommError(handler, &err, &st);
}

void AsyncSerial::_read_async()
{
    DWORD anyerr{};
    char smolbuf[64]{};
    size_t readdin = 0;

    while (keep_running && !anyerr && !st.fEof) {
        DWORD readd{};

        if (!_upd_stat()) {
            keep_running = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(125));
            std::this_thread::yield();
            continue;
        }

        if (!(readdin = static_cast<size_t>(st.cbInQue))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(125));
            std::this_thread::yield();
            continue;
        }

        readdin = readdin > std::size(smolbuf) ? std::size(smolbuf) : readdin;

        if (ReadFile(handler, smolbuf, static_cast<DWORD>(readdin), &readd, NULL)) {
            std::lock_guard<std::mutex> l(buf_m);
            const size_t fix = buf.size() + readd > max_buffer_len ? (max_buffer_len - buf.size()) : readd;
            buf.insert(buf.end(), std::begin(smolbuf), std::begin(smolbuf) + static_cast<size_t>(readd));
        }

        buf_c.notify_one();
    }
    if (anyerr) {
        keep_running = false;        
    }
}

AsyncSerial::AsyncSerial(const size_t max)
    : max_buffer_len(max)
{
}

AsyncSerial::~AsyncSerial()
{
    close();
}

CommStatus AsyncSerial::try_connect_port(const size_t comm, const DWORD baudrate)
{
    const auto portnam = ("\\\\.\\COM" + std::to_string(comm));
    close();
    handler = CreateFileA(static_cast<LPCSTR>(portnam.c_str()), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!handler) return CommStatus::FAILED;

    DCB dcbSerialParameters{};
    if (!GetCommState(handler, &dcbSerialParameters)) {
        CloseHandle(handler);
        handler = nullptr;
        return CommStatus::FAILED;
    }

    dcbSerialParameters.BaudRate = baudrate;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = NOPARITY;
    dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(handler, &dcbSerialParameters)) {
        CloseHandle(handler);
        handler = nullptr;
        return CommStatus::FAILED;
    }

    PurgeComm(handler, PURGE_RXCLEAR | PURGE_TXCLEAR);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    _upd_stat();
    const size_t buff = static_cast<size_t>(st.cbInQue);

    keep_running = true;
    thr = std::thread([this] {_read_async(); });
    
    if (buff) return CommStatus::READY_HASBUF;
    return CommStatus::READY_FRESH;
}

bool AsyncSerial::has_data(const uint64_t ms)
{
    std::unique_lock<std::mutex> l(buf_m);
    for(uint64_t p = 0; p < ms / 100; ++p) buf_c.wait_for(l, std::chrono::milliseconds(100), [this] {return (!valid()) || buf.size(); });
    return buf.size();
}

bool AsyncSerial::valid() const
{
    return handler != nullptr && keep_running;
}

AsyncSerial::operator bool() const
{
    return valid();
}

std::string AsyncSerial::getline(const bool block)
{
    if (!handler) return {};

    if (!block) {
        if (buf.empty()) return {};
    } 

    std::unique_lock<std::mutex> l(buf_m);

    auto p = buf.end();
    buf_c.wait_for(l, std::chrono::milliseconds(200), [this, &p] {return (!valid()) || (p = std::find(buf.begin(), buf.end(), '\n')) != buf.end(); });
    if (p == buf.end() || !valid()) return {};
    
    std::string popped{buf.begin(), p};
    buf.erase(buf.begin(), p);
    while (buf.size() && buf.front() == '\n') buf.erase(buf.begin());
    return popped;
}

std::string AsyncSerial::read(const size_t reada, const bool block)
{
    if (!handler) return {};

    if (!block) {
        if (buf.empty()) return {};
    }

    std::unique_lock<std::mutex> l(buf_m);

    buf_c.wait_for(l, std::chrono::milliseconds(200), [this, &reada] {return (!valid()) || buf.size() >= reada; });
    if (buf.size() < reada || !valid()) return {};

    std::string popped{ buf.begin(), buf.begin() + reada };
    buf.erase(buf.begin(), buf.begin() + reada);
    return popped;
}

size_t AsyncSerial::write(const std::string& str)
{
    if (!handler) return 0;

    DWORD _sent{};
    if (!WriteFile(handler, str.data(), static_cast<DWORD>(str.size()), &_sent, NULL)) {
        close();
        return 0;
    }
    return static_cast<size_t>(_sent);
}

void AsyncSerial::close()
{
    keep_running = false;
    if (thr.joinable()) thr.join();
    std::lock_guard<std::mutex> l(buf_m);
    if (handler) CloseHandle(handler);
    handler = nullptr;
}
