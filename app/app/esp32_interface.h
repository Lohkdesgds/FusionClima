#pragma once

#include "deps/SerialPort.hpp"
#include "protocol_combo.h"

constexpr size_t com_max_port = 120;
constexpr size_t esp32_buf_max = sizeof(Comm::usb_format_raw) * 4;

class ESP32_interface {
public:
	enum class commands {REQUEST};

	bool search_and_setup(const bool forcereset = false);
	void send_command(const commands&);

	Comm::usb_format_raw get() const;
	std::chrono::system_clock::time_point get_last_update() const;

	operator bool() const;
	bool valid() const;
private:
	AsyncSerial m_async{ esp32_buf_max };
	size_t m_port = 0;

	mutable std::mutex m_raw_data_mtx;
	Comm::usb_format_raw m_raw_data_buf;
	std::chrono::system_clock::time_point m_last_update = std::chrono::system_clock::now();
};