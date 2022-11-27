#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include <shellapi.h>

#include <httplib.h>

#include "esp32_interface.h"
#include "display.h"
#include "handlers.h"

constexpr float expected_time_sec = 60.0f;

void async_screen(Display& dsp, ESP32_interface& esp)
{
	dsp.create_and_load();
	al_install_mouse();
	auto& ddat = dsp.get();

	ALLEGRO_EVENT_QUEUE* evqu = al_create_event_queue();
	al_register_event_source(evqu, dsp.get_event());
	al_register_event_source(evqu, al_get_mouse_event_source());

	std::chrono::system_clock::time_point last_invalid = std::chrono::system_clock::now();

	while (dsp) {
		if (esp) {
			if (std::chrono::system_clock::now() - last_invalid > std::chrono::seconds(3)) {
				const float currt = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - esp.get_last_update()).count() * 0.001f);
				const float calct = currt * 1.0f / expected_time_sec;
				ddat.bar_complete = calct < 0.0f ? 0.0f : (calct > 1.0f ? 1.0f : calct);
			}
		}
		else {
			ddat.bar_complete = 1.0f;
			last_invalid = std::chrono::system_clock::now();
		}

		dsp.draw();

		ALLEGRO_EVENT ev;
		if (!al_wait_for_event_timed(evqu, &ev, 1.0 / 61)) continue;

		switch (ev.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			dsp.destroy();
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:

			if (ev.mouse.x >= static_cast<int>(20.00f * display_orig_size_scale[0]) && ev.mouse.y >= static_cast<int>(140.0f * display_orig_size_scale[1]) &&
				ev.mouse.x <= static_cast<int>(260.0f * display_orig_size_scale[0]) && ev.mouse.y <= static_cast<int>(380.0f * display_orig_size_scale[1]))
			{ // left rectangle: ABRIR HTML
				std::cout << "Open HTML\n";
				ShellExecute(0, 0, L"http://127.0.0.1", 0, 0, SW_SHOW);
			}

			if (esp && 
				ev.mouse.x >= static_cast<int>(440.0f * display_orig_size_scale[0]) && ev.mouse.y >= static_cast<int>(140.0f * display_orig_size_scale[1]) &&
				ev.mouse.x <= static_cast<int>(680.0f * display_orig_size_scale[0]) && ev.mouse.y <= static_cast<int>(380.0f * display_orig_size_scale[1]))
			{ // right rectangle: REFRESH
				std::cout << "Refresh\n";
				esp.send_command(ESP32_interface::commands::REQUEST);
			}
			break;
		}
	}
}

void async_website(Display& dsp, ESP32_interface& esp)
{
	while (!dsp) std::this_thread::sleep_for(std::chrono::seconds(1));

	httplib::Server svr;

	svr.set_keep_alive_max_count(10);
	//svr.set_post_routing_handler(post_routing_handler);
	svr.set_error_handler(error_handler);
	svr.set_exception_handler(exception_handler);
	svr.set_logger(static_logger);
	svr.set_pre_routing_handler([&](const httplib::Request& a, httplib::Response& b) {return pre_router_handler(a, b, dsp, esp); });
	svr.Get(".*", file_request_handler);

	svr.listen("0.0.0.0", 80);

	while (dsp) std::this_thread::sleep_for(std::chrono::seconds(1));

	svr.stop();
}

int main()
{
	ESP32_interface esp;
	Display disp;
	auto& ddat = disp.get();

	disp.set_title("FusionClima V1.0");
	disp.set_end_title("Loading...");
	

	std::thread thr_disp([&] {async_screen(disp, esp); });
	std::thread thr_web([&] {async_website(disp, esp); });

	while (!disp) Sleep(100);

	//disp.draw();

	while (disp) {
		disp.set_end_title("Trying to connect...");
		ddat.connected = false;
		ddat.bar_complete = 1.0f;

		//disp.draw();
		std::cout << "Connecting...\n";
		while (disp && !esp.search_and_setup(true));
		if (!disp) break;

		disp.set_end_title("Connected");
		std::cout << "Connected\n";
		ddat.connected = true;
		//disp.draw();

		while (esp && disp) {
			//const auto usb_data = esp.get();
			//
			//const float currt = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - esp.get_last_update()).count() * 0.001f);
			////const float calct = currt * 1.0f / expected_time_sec;
			//
			//printf_s("\nData:\nRSSI=%.2f SNR=%.2f\nTemperature: %.1f\nHumidity: %.1f\nSource: %llX\nID: %i\n",
			//	usb_data.rssi_x10 * 0.1f, usb_data.snr_x10 * 0.1f, usb_data.data.data.th.temp_x10 * 0.1f, usb_data.data.data.th.hum_x10 * 0.1f, usb_data.data.ident, (unsigned)usb_data.pck_id);
			//std::cout << "Last update: " << currt << " s" << std::endl;
			//
			////ddat.bar_complete = calct < 0.0f ? 0.0f : (calct > 1.0f ? 1.0f : calct);
			////disp.draw();
		
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}

		disp.set_end_title("Lost connection to dongle.");
		std::cout << "Disconnected\n";
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	thr_disp.join();
	thr_web.join();

	return 0;
}