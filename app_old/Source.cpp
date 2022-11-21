#define WIN32_LEAN_AND_MEAN
#include "sources/fusion_clima.h" // image
#include "deps/SerialPort.hpp"
#include "../fc/protocol_combo.h"
//#include <Graphics.h>
//#include <System.h>
#include <memory>
#include <unordered_map>

int main(int argc, char* argv)
{

}


//using namespace AllegroCPP;
//
//enum class pred_menus : uint16_t {NONE, EXIT};
//enum class states_draw : uint16_t {LAUNCHING, COMMUNICATION};
//
//const std::unordered_map<states_draw, double> _autotime = {
//	{states_draw::LAUNCHING, 5.0},
//	{states_draw::COMMUNICATION, 0.0}
//};
//
//constexpr float fix(const float f) { return 0.5f + 0.5f * f; }
//double gettime(const states_draw& d) { const auto i = _autotime.find(d); if (i != _autotime.end()) return i->second; return 0.0; }


//int main()
//{
//	Display disp({720, 480}, "FusionApp");
//	Event_queue evqu;
//	Font font;
//	File_tmp fp_fusionclima;
//	
//	fp_fusionclima.write(Sources::png_fusionclima, std::size(Sources::png_fusionclima));
//	fp_fusionclima.flush();
//	
//	Bitmap bmp_fc(fp_fusionclima);
//	
//	states_draw stat = states_draw::LAUNCHING;
//	double stat_time_ctl = 0.0;
//	
//	auto dt = std::chrono::high_resolution_clock::now();
//	double smooth = 0.0;
//	
//	evqu << disp;
//	
//	while (disp.valid()) {
//		if (const auto ev = evqu.get_next_event(); ev)
//		{
//			switch (ev.get().type) {
//			case ALLEGRO_EVENT_DISPLAY_CLOSE:
//				disp.destroy();
//				break;
//			}
//		}
//		else {
//			switch (stat) {
//			case states_draw::LAUNCHING:
//				if (Time::get_time() - stat_time_ctl > gettime(stat)){
//					stat = states_draw::COMMUNICATION;
//				}
//				else {
//	
//				}
//				break;
//			case states_draw::COMMUNICATION:
//			{
//				disp.clear_to_color(al_map_rgb_f(fix(std::sin(Time::get_time())), fix(std::cos(0.85f * Time::get_time())), fix(std::sin(0.45f + 1.1f * Time::get_time()))));
//	
//			}
//				break;
//			}
//	
//			const double weight = 0.5 * fabs(smooth) + 1.0;
//			smooth = (weight * smooth + 1.0 / std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(std::chrono::high_resolution_clock::now() - std::exchange(dt, std::chrono::high_resolution_clock::now())).count()) / (weight + 1.0);
//			font.draw({ 0.0f,0.0f }, ("FPS: " + std::to_string(smooth)));
//	
//			disp.flip();
//		}
//	}
//
//}

//void amain(bool&);
//
//int main(bool&)
//{
//	bool ctl = false;
//
//	size_t count_nofails = 0;
//
//	std::thread thr([&] {
//		while (1) {
//			std::cout << "[CTL] Waiting...\n";
//			while (!ctl) std::this_thread::sleep_for(std::chrono::milliseconds(100));
//			std::cout << "[CTL] Timeout in 3 sec...\n";
//			std::this_thread::sleep_for(std::chrono::seconds(3));
//			ctl = false;
//			std::cout << "[CTL] Times: " << ++count_nofails << "\n";
//		}
//	});
//
//	std::this_thread::sleep_for(std::chrono::seconds(1));
//
//	while (1) {
//		amain(ctl);
//	}
//}
//
//void amain(bool& keep_runn)
//{
//	Menu menu({ Menu_each_menu("File", 0, { Menu_each_default("Exit", static_cast<uint16_t>(pred_menus::EXIT))}) });
//	keep_runn = true;
//	bool thr_conf = false;
//	Comm::usb_format_raw usb_data;
//
//	Thread thr([&] {
//		Display disp({ 1280, 720 }, "FusionClima App");
//		Font font;
//		thr_conf = true;
//
//		{
//			Event_queue evqu;
//
//			evqu << disp;
//			evqu << menu;
//
//			menu >> disp;
//
//			while (keep_runn) {
//
//				auto ev = evqu.get_next_event();
//
//				if (ev) {
//					switch (ev.get().type) {
//					case ALLEGRO_EVENT_DISPLAY_CLOSE:
//						keep_runn = false;
//						break;
//					case ALLEGRO_EVENT_MENU_CLICK:
//					{
//						Menu_event mev(ev);
//						switch (mev.get_id()) {
//						case static_cast<uint16_t>(pred_menus::EXIT):
//							keep_runn = false;
//							break;
//						default:
//							break;
//						}
//					}
//					break;
//					default:
//						break;
//					}
//				}
//
//				font.draw({ 0.0f,10.0f }, "Test");
//				disp.flip();
//			}
//		}
//		
//		return false;
//	});
//
//	thr.start();
//	while (!thr_conf) std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//	AsyncSerial serial(sizeof(Comm::usb_format_raw) * 4);
//
//	while (keep_runn) {
//		std::cout << "Searching..." << std::endl;
//
//		for (uint32_t p = 0; p < 120; ++p) {
//			if (serial.try_connect_port(p) != CommStatus::FAILED) {
//				{
//					if (!serial.has_data(600)) continue;
//
//					auto _src = serial.read(sizeof(Comm::usb_format_raw), false);
//					if (_src.empty() || _src.size() != sizeof(Comm::usb_format_raw)) continue;
//
//					Comm::usb_format_raw& cas = *((Comm::usb_format_raw*)_src.data());
//					if (strcmp(cas.trigg, Comm::protID) != 0) continue;
//				}
//
//				std::cout << "Connected to port #" << p << std::endl;
//
//				while (serial.valid() && keep_runn)
//				{
//					auto _src = serial.read(sizeof(Comm::usb_format_raw));
//					if (_src.empty() || _src.size() != sizeof(Comm::usb_format_raw)) {
//						std::this_thread::sleep_for(std::chrono::milliseconds(250));
//						continue;
//					}
//
//					memcpy(&usb_data, _src.data(), sizeof(usb_data));
//
//					printf_s("\n\nData:\nRSSI=%.2f SNR=%.2f\nTemperature: %.1f\nHumidity: %.1f\nSource: %llX\nID: %i",
//						usb_data.rssi_x10 * 0.1f, usb_data.snr_x10 * 0.1f, usb_data.data.temp_x10 * 0.1f, usb_data.data.hum_x10 * 0.1f, usb_data.data.ident, (unsigned)usb_data.pck_id);
//				}
//
//
//				std::cout << "Disconnected" << std::endl;
//			}
//		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
//	}
//
//	thr.join();
//}