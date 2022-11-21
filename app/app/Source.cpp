#include <iostream>
#include <string>
#include <thread>

#include "esp32_interface.h"
#include "display.h"

constexpr float expected_time_sec = 60.0f;

void async_screen(Display& dsp, ESP32_interface& esp)
{
	dsp.create_and_load();
	auto& ddat = dsp.get();

	ALLEGRO_EVENT_QUEUE* evqu = al_create_event_queue();
	al_register_event_source(evqu, dsp.get_event());

	while (dsp) {
		if (esp) {
			const float currt = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - esp.get_last_update()).count() * 0.001f);
			const float calct = currt * 1.0f / expected_time_sec;
			ddat.bar_complete = calct < 0.0f ? 0.0f : (calct > 1.0f ? 1.0f : calct);
		}
		else ddat.bar_complete = 1.0f;

		dsp.draw();

		ALLEGRO_EVENT ev;
		if (!al_wait_for_event_timed(evqu, &ev, 1.0 / 61)) continue;

		switch (ev.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			dsp.destroy();
			break;
		}
	}
}

int main()
{
	ESP32_interface esp;
	Display disp;
	auto& ddat = disp.get();

	disp.set_title("FusionClima V1.0");
	disp.set_end_title("Loading...");
	

	std::thread thr([&] {async_screen(disp, esp); });
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
			const auto usb_data = esp.get();
			
			const float currt = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - esp.get_last_update()).count() * 0.001f);
			//const float calct = currt * 1.0f / expected_time_sec;
		
			printf_s("\nData:\nRSSI=%.2f SNR=%.2f\nTemperature: %.1f\nHumidity: %.1f\nSource: %llX\nID: %i\n",
				usb_data.rssi_x10 * 0.1f, usb_data.snr_x10 * 0.1f, usb_data.data.data.th.temp_x10 * 0.1f, usb_data.data.data.th.hum_x10 * 0.1f, usb_data.data.ident, (unsigned)usb_data.pck_id);
			std::cout << "Last update: " << currt << " s" << std::endl;
		
			//ddat.bar_complete = calct < 0.0f ? 0.0f : (calct > 1.0f ? 1.0f : calct);
			//disp.draw();
		
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}

		disp.set_end_title("Lost connection to dongle.");
		std::cout << "Disconnected\n";
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	thr.join();

	return 0;
}