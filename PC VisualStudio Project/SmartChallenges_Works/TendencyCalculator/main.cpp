#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "../shared/datahandler.h"
#include "algorithm_test.h"

//#define ENABLE_DRAW
#define ENABLE_TEST

const std::string inuse = "../shared/data_transf.txt";
const std::string fontpath = "../shared/segoeuib.ttf";
const std::string defexport = "plot_";
const size_t bmpsiz = 5000;
const size_t bmpbord = 150;
const float transparency = 0.05f;
const float saturation = transparency;// 1.0f;
const float thicc = 12.0f, thicc_avg = 8.0f;
const double minchuva = 5.0;
const size_t fontsiz = 50;

int main()
{
	std::cout << "Loading file..." << std::endl;
	const auto things = readfile(inuse);
	if (things.empty()) return 1;
	std::cout << "Translating stuff..." << std::endl;
	const auto things2 = get_preds_steps(things, minchuva);
	std::cout << "Got #" << things2.size() << " entities" << std::endl;

#ifdef ENABLE_TEST
	std::cout << "\n\nTesting algorithm..." << std::endl;

	long double score_correct = 0.0;

	for (const auto& ea : things2) {
		const double got = chances_of(ea);
		score_correct += static_cast<long double>((got - 0.7) * (got < 0.7 ? 5.0 : 1.0)); // penalty 5x if bad, else ++
	}

	std::cout << "Algorithm got score of correctness: " << score_correct << " of " << things2.size() << " entities" << std::endl;

	//for (const auto& things) {
	//
	//}

#endif
#ifdef ENABLE_DRAW

	ALLEGRO_DISPLAY* dsp = nullptr;

	ALLEGRO_FONT* font = nullptr;
	
	ALLEGRO_BITMAP* bmp[3] = { nullptr, nullptr, nullptr };
	ALLEGRO_BITMAP* interm = nullptr;
	double scale_range_bmp[3][2]{};

	ALLEGRO_EVENT_QUEUE* evqu = nullptr;
	size_t bmp_select = 0;

	std::cout << "Starting draw stuff..." << std::endl;

	al_init();
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_install_keyboard();

	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

	dsp = al_create_display(1200, 1200);
	evqu = al_create_event_queue();
	bmp[0] = al_create_bitmap(bmpsiz, bmpsiz);
	bmp[1] = al_create_bitmap(bmpsiz, bmpsiz);
	bmp[2] = al_create_bitmap(bmpsiz, bmpsiz);
	interm = al_create_bitmap(bmpsiz + 2 * bmpbord, bmpsiz + 2 * bmpbord);
	font = al_load_ttf_font(fontpath.c_str(), fontsiz, 0);

	if (!dsp || !evqu || !bmp[0] || !bmp[1] || !bmp[2]) return 2;

	al_register_event_source(evqu, al_get_display_event_source(dsp));
	al_register_event_source(evqu, al_get_keyboard_event_source());

	al_set_target_backbuffer(dsp);
	al_clear_to_color(al_map_rgb(25, 25, 25));
	al_flip_display();

	std::cout << "Plotting..." << std::endl;

	al_set_target_bitmap(bmp[0]);
	{
		double miny = 60.0, maxy = 10.0;
		for (const auto& i : things2) {
			for (const auto& j : i.mem) {
				if (j.temp < miny && j.temp >= 10.0) miny = j.temp;
				if (j.temp > maxy && j.temp <= 60.0) maxy = j.temp;
			}
		}
		miny -= 0.2;
		if (maxy < miny) maxy = miny + 1.0;
		maxy += 0.2;

		double avg[pred_size]{};

		for (const auto& i : things2) {
			for (size_t p = 0; p < (pred_size - 1); ++p) {
				avg[p] += ((i.mem[p].temp - miny) / (maxy - miny)) * 1.0 / things2.size();
				const float b4 = static_cast<float>((i.mem[p].temp - miny) / (maxy - miny)) * bmpsiz;
				const float af = static_cast<float>((i.mem[p + 1].temp - miny) / (maxy - miny)) * bmpsiz;

				const float x1 = static_cast<float>(static_cast<double>(p) * 1.0 / (pred_size - 1)) * bmpsiz;
				const float x2 = static_cast<float>(static_cast<double>(p + 1) * 1.0 / (pred_size - 1)) * bmpsiz;
				al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgba_f(saturation, 0.0f, 0.0f, transparency), thicc);
			}
			avg[(pred_size - 1)] += ((i.mem[(pred_size - 1)].temp - miny) / (maxy - miny)) * 1.0 / things2.size();
		}
		for (size_t p = 0; p < (pred_size - 1); ++p) {
			const float b4 = avg[p] * bmpsiz;
			const float af = avg[p + 1] * bmpsiz;

			const float x1 = static_cast<float>(static_cast<double>(p) * 1.0 / (pred_size - 1)) * bmpsiz;
			const float x2 = static_cast<float>(static_cast<double>(p + 1) * 1.0 / (pred_size - 1)) * bmpsiz;
			al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgba_f(1.0f, 1.0f, 1.0f, 0.2f), thicc_avg);
		}

		scale_range_bmp[0][0] = miny;
		scale_range_bmp[0][1] = maxy;
	}

	al_set_target_bitmap(bmp[1]);
	{
		double miny = 100.0, maxy = -50.0;
		for (const auto& i : things2) {
			for (const auto& j : i.mem) {
				if (j.umid < miny && j.umid >= -50.0) miny = j.umid;
				if (j.umid > maxy && j.umid <= 100.0) maxy = j.umid;
			}
		}
		miny -= 0.2;
		if (maxy < miny) maxy = miny + 1.0;
		maxy += 0.2;

		double avg[pred_size]{};

		for (const auto& i : things2) {
			for (size_t p = 0; p < (pred_size - 1); ++p) {
				avg[p] += ((i.mem[p].umid - miny) / (maxy - miny)) * 1.0 / things2.size();

				const float b4 = static_cast<float>((i.mem[p].umid - miny) / (maxy - miny)) * bmpsiz;
				const float af = static_cast<float>((i.mem[p + 1].umid - miny) / (maxy - miny)) * bmpsiz;

				const float x1 = static_cast<float>(static_cast<double>(p) * 1.0 / (pred_size - 1)) * bmpsiz;
				const float x2 = static_cast<float>(static_cast<double>(p + 1) * 1.0 / (pred_size - 1)) * bmpsiz;
				al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgba_f(0.0f, saturation, 0.0f, transparency), thicc);
			}
			avg[(pred_size - 1)] += ((i.mem[(pred_size - 1)].umid - miny) / (maxy - miny)) * 1.0 / things2.size();
		}
		for (size_t p = 0; p < (pred_size - 1); ++p) {
			const float b4 = avg[p] * bmpsiz;
			const float af = avg[p + 1] * bmpsiz;

			const float x1 = static_cast<float>(static_cast<double>(p) * 1.0 / (pred_size - 1)) * bmpsiz;
			const float x2 = static_cast<float>(static_cast<double>(p + 1) * 1.0 / (pred_size - 1)) * bmpsiz;
			al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgba_f(1.0f, 1.0f, 1.0f, 0.2f), thicc_avg);
		}
		scale_range_bmp[1][0] = miny;
		scale_range_bmp[1][1] = maxy;
	}

	al_set_target_bitmap(bmp[2]);
	{
		double miny = 100.0, maxy = 0.0;
		for (const auto& i : things2) {
			for (const auto& j : i.mem) {
				if (j.prec_chuva < miny && j.prec_chuva >= 0.0) miny = j.prec_chuva;
				if (j.prec_chuva > maxy && j.prec_chuva <= 100.0) maxy = j.prec_chuva;
			}
		}
		miny -= 0.2;
		if (maxy < miny) maxy = miny + 1.0;
		maxy += 0.2;

		double avg[pred_size]{};

		for (const auto& i : things2) {
			for (size_t p = 0; p < (pred_size - 1); ++p) {
				avg[p] += ((i.mem[p].prec_chuva - miny) / (maxy - miny)) * 1.0 / things2.size();
				const float b4 = static_cast<float>((i.mem[p].prec_chuva - miny) / (maxy - miny)) * bmpsiz;
				const float af = static_cast<float>((i.mem[p + 1].prec_chuva - miny) / (maxy - miny)) * bmpsiz;

				const float x1 = static_cast<float>(static_cast<double>(p) * 1.0 / (pred_size - 1)) * bmpsiz;
				const float x2 = static_cast<float>(static_cast<double>(p + 1) * 1.0 / (pred_size - 1)) * bmpsiz;
				al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgba_f(0.0f, 0.0f, saturation, transparency), thicc);
			}
			avg[(pred_size - 1)] += ((i.mem[(pred_size - 1)].prec_chuva - miny) / (maxy - miny)) * 1.0 / things2.size();
		}
		for (size_t p = 0; p < (pred_size - 1); ++p) {
			const float b4 = avg[p] * bmpsiz;
			const float af = avg[p + 1] * bmpsiz;

			const float x1 = static_cast<float>(static_cast<double>(p) * 1.0 / (pred_size - 1)) * bmpsiz;
			const float x2 = static_cast<float>(static_cast<double>(p + 1) * 1.0 / (pred_size - 1)) * bmpsiz;
			al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgba_f(1.0f, 1.0f, 1.0f, 0.2f), thicc_avg);
		}
		scale_range_bmp[2][0] = miny;
		scale_range_bmp[2][1] = maxy;
	}

	const auto update_screen = [&](size_t selecc, bool draww = true) {
		al_set_target_bitmap(interm);
		const int ix = al_get_bitmap_width(interm);
		const int iy = al_get_bitmap_height(interm);
		al_clear_to_color(al_map_rgb(80, 80, 80));

		switch (selecc) {
		case 0:
			al_draw_textf(font, al_map_rgb(255, 255, 255), 5, 0, 0, (char*)u8"Temperature [8 hours period] [ºC] (filtered >= %.2lf mm rain)", minchuva);
			break;
		case 1:
			al_draw_textf(font, al_map_rgb(255, 255, 255), 5, 0, 0, (char*)u8"Umidity [8 hours period] [%%] (filtered >= %.2lf mm rain)", minchuva);
			break;
		case 2:
			al_draw_textf(font, al_map_rgb(255, 255, 255), 5, 0, 0, (char*)u8"Rain [8 hours period] [mm] (filtered >= %.2lf mm rain)", minchuva);
			break;
		default:
			al_draw_textf(font, al_map_rgb(255, 255, 255), 5, 0, 0, (char*)u8"You broke the app lmao!");
			return;
		}//al_get_font_line_height(font)
		al_draw_filled_rectangle(bmpbord, bmpbord, ix - bmpbord, iy - bmpbord, al_map_rgb(16, 16, 16));
		al_draw_scaled_bitmap(bmp[selecc], 0, 0, bmpsiz, bmpsiz, bmpbord, bmpbord, ix - 2 * bmpbord, iy - 2 * bmpbord, 0);

		const float y_step = static_cast<float>(iy - 2 * bmpbord) / static_cast<float>(pred_size - 1);
		const float x_step = static_cast<float>(ix - 2 * bmpbord) / static_cast<float>(pred_size - 1);
		const float y_begin = bmpbord - al_get_font_line_height(font) * 0.5f;
		const float x_begin = bmpbord; // - strlen

		const auto& currr = scale_range_bmp[selecc];

		switch (selecc) {
		case 0:
		{
			for (size_t p = 0; p < pred_size; ++p) {
				double calc = currr[1] - (currr[1] - currr[0]) * (static_cast<double>(p) / static_cast<double>(pred_size - 1));
				al_draw_textf(font, al_map_rgb(255, 255, 255), 0.88 * bmpbord, y_begin + y_step * static_cast<float>(p), ALLEGRO_ALIGN_RIGHT, (char*)u8"%.2lf", calc);
				al_draw_textf(font, al_map_rgb(255, 255, 255), x_begin + x_step * static_cast<float>(p), iy - 0.92 * bmpbord, ALLEGRO_ALIGN_CENTER, (std::to_string(-(static_cast<int64_t>(pred_size - p) - 1)) + "h").c_str());
			}
		}
			break;
		case 1:
		{
			for (size_t p = 0; p < pred_size; ++p) {
				double calc = currr[1] - (currr[1] - currr[0]) * (static_cast<double>(p) / static_cast<double>(pred_size - 1));
				al_draw_textf(font, al_map_rgb(255, 255, 255), 0.88 * bmpbord, y_begin + y_step * static_cast<float>(p), ALLEGRO_ALIGN_RIGHT, (char*)u8"%.2lf", calc);
				al_draw_textf(font, al_map_rgb(255, 255, 255), x_begin + x_step * static_cast<float>(p), iy - 0.92 * bmpbord, ALLEGRO_ALIGN_CENTER, (std::to_string(-(static_cast<int64_t>(pred_size - p) - 1)) + "h").c_str());
			}
		}
			break;
		case 2:
		{
			for (size_t p = 0; p < pred_size; ++p) {
				double calc = currr[1] - (currr[1] - currr[0]) * (static_cast<double>(p) / static_cast<double>(pred_size - 1));
				al_draw_textf(font, al_map_rgb(255, 255, 255), 0.88 * bmpbord, y_begin + y_step * static_cast<float>(p), ALLEGRO_ALIGN_RIGHT, (char*)u8"%.2lf", calc);
				al_draw_textf(font, al_map_rgb(255, 255, 255), x_begin + x_step * static_cast<float>(p), iy - 0.92 * bmpbord, ALLEGRO_ALIGN_CENTER, (std::to_string(-(static_cast<int64_t>(pred_size - p) - 1)) + "h").c_str());
			}
		}
			break;
		}
		if (draww) {
			al_set_target_backbuffer(dsp);
			al_draw_scaled_bitmap(interm, 0, 0, ix, iy, 0, 0, al_get_display_width(dsp), al_get_display_height(dsp), 0);
			al_flip_display();
		}
	};

	std::cout << "Saving into disk..." << std::endl;

	for (size_t p = 0; p < 3; ++p) {
		update_screen(p, false);
		al_save_bitmap((defexport + std::to_string(p) + ".png").c_str(), interm);
	}

	std::cout << "Rendering..." << std::endl;

	update_screen(bmp_select);

	std::cout << "Ready." << std::endl;

	for (bool run = true; run;) {
		ALLEGRO_EVENT ev{};
		al_wait_for_event(evqu, &ev);

		switch (ev.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			run = false;
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			switch (ev.keyboard.keycode) {
			case ALLEGRO_KEY_ESCAPE:
				run = false;
				break;
			case ALLEGRO_KEY_SPACE:
				bmp_select = (bmp_select + 1) % 3;
				std::cout << "Rendering..." << std::endl;
				update_screen(bmp_select);
				std::cout << "Selected: " << bmp_select << std::endl;
				break;
			}
			break;
		}
	}

	for (auto& i : bmp) al_destroy_bitmap(i);
	al_destroy_bitmap(interm);
	al_destroy_font(font);
	al_destroy_event_queue(evqu);
	al_destroy_display(dsp);

#endif

	return 0;
}