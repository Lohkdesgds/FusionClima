#include "deps/SerialPort.hpp"
#include "../fc/protocol_combo.h"

int main()
{
	AsyncSerial serial(sizeof(Comm::usb_format_raw) * 4);

	while (1) {
		std::cout << "Searching..." << std::endl;

		for (uint32_t p = 0; p < 120; ++p) {
			if (serial.try_connect_port(p) != CommStatus::FAILED) {
				{
					if (!serial.has_data(600)) continue;

					auto _src = serial.read(sizeof(Comm::usb_format_raw), false);
					if (_src.empty() || _src.size() != sizeof(Comm::usb_format_raw)) continue;

					Comm::usb_format_raw& cas = *((Comm::usb_format_raw*)_src.data());
					if (strcmp(cas.trigg, Comm::protID) != 0) continue;
				}

				std::cout << "Connected to port #" << p << std::endl;

				while (serial.valid())
				{
					auto _src = serial.read(sizeof(Comm::usb_format_raw));
					if (_src.empty()) {
						std::this_thread::sleep_for(std::chrono::milliseconds(250));
						continue;
					}

					Comm::usb_format_raw& cas = *((Comm::usb_format_raw*)_src.data());

					printf_s("\n\nData:\nRSSI=%.2f SNR=%.2f\nTemperature: %.1f\nHumidity: %.1f\nSource: %llX\nID: %i",
						cas.rssi_x10 * 0.1f, cas.snr_x10 * 0.1f, cas.data.temp_x10 * 0.1f, cas.data.hum_x10 * 0.1f, cas.data.ident, (unsigned)cas.pck_id);

					//std::cout << "Got data from: " << std::hex << cas.data.ident << std::dec
					//	<< "\nRSSI: " << cas.rssi << "\nSNR: " << cas.snr
					//	<< "\nTemperature: " << (0.1f * cas.data.temp_x10)
					//	<< "\nHumidity: " << (0.1f * cas.data.hum_x10) << std::endl;
				}

				std::cout << "Disconnected" << std::endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
	}
}