#include "esp32_interface.h"

bool ESP32_interface::search_and_setup(const bool forcereset)
{
	if (m_async.valid() && !forcereset) return true;
	m_async.close();
	m_async.hook_rwbuf({}); // unhook

	for (m_port = 0; m_port <= com_max_port; ++m_port)
	{
		std::cout << m_port << "  \r";
		if (m_async.try_connect_port(m_port) == CommStatus::FAILED) continue;

		{
			if (!m_async.has_data(500)) {
				m_async.close();
				continue;
			}
		
			auto _src = m_async.read(sizeof(Comm::usb_format_raw), false);
			if (_src.empty() || _src.size() != sizeof(Comm::usb_format_raw)) continue;
		
			Comm::usb_format_raw& cas = *((Comm::usb_format_raw*)_src.data());
			if (strcmp(cas.trigg, Comm::protID) != 0) continue;
		}
		
		m_async.hook_rwbuf([this] (std::vector<char>& _src) {
			if (_src.size() < sizeof(Comm::usb_format_raw)) return;
			std::lock_guard<std::mutex> l(m_raw_data_mtx);
			const auto lastid = m_raw_data_buf.pck_id;
			memcpy(&m_raw_data_buf, _src.data(), sizeof(m_raw_data_buf));
			if (lastid != m_raw_data_buf.pck_id) m_last_update = std::chrono::system_clock::now();
			_src.erase(_src.begin(), _src.begin() + sizeof(m_raw_data_buf));
		});

		std::cout << "[Debug] Connected to port #" << m_port << std::endl;
		break;
	}

	return m_async.valid();
}

void ESP32_interface::send_command(const commands& c)
{
	switch (c) {
	case commands::REQUEST:
		m_async.write("request\n");
		break;
	}
}

Comm::usb_format_raw ESP32_interface::get() const
{
	std::lock_guard<std::mutex> l(m_raw_data_mtx);
	return m_raw_data_buf;
}

std::chrono::system_clock::time_point ESP32_interface::get_last_update() const
{
	std::lock_guard<std::mutex> l(m_raw_data_mtx);
	return m_last_update;
}

ESP32_interface::operator bool() const
{
	return m_async.valid();
}

bool ESP32_interface::valid() const
{
	return m_async.valid();
}
