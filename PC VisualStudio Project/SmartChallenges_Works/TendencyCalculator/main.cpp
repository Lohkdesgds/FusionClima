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
#include "paralelism.h"

//#define ENABLE_DRAW
#define ENABLE_TEST

const std::string inuse = "../shared/data_transf.txt";
const std::string fontpath = "../shared/segoeuib.ttf";
const std::string defexport = "plot_";
const size_t bmpsiz = 5000;
const size_t bmpbord = 275;
const float transparency = 0.05f;
const float saturation = transparency;// 1.0f;
const float thicc = 60.0f, thicc_avg = 80.0f;
const double minchuva = 5.0;
const size_t fontsiz = 85;
const size_t thr_count = 16;
const size_t thr_steps = 6;
const double thr_precision = 1.0 / (thr_count * thr_steps);

void recursive_loop_of(const size_t (&slice_first)[2], const double& pointa, const double& pointb, const double& prec, double(&Ks)[num_ks], const std::function<void(const double(&Ks)[num_ks])>& f, const size_t off = num_ks)
{
	if (!f) return;

	const double range = (pointb - pointa) * 1.0 / (slice_first[1] != 0 ? slice_first[1] : 1.0);
	const double first = range * ((slice_first[1] == 0) ? 0.0 : slice_first[0]);

	for (double K = first; K < first + range; K += prec)
	{
		Ks[off - 1] = K;
		if (off > 1) recursive_loop_of({0, 0}, pointa, pointb, prec, Ks, f, off - 1);
		else f(Ks);
	}
}

int main()
{
	const auto printdb = [](double d) {char tmp[128]; sprintf_s(tmp, "%.8lf", d); return std::string(tmp); };
	const auto printdbs = [](double d) {char tmp[128]; sprintf_s(tmp, "%.4lf", d); return std::string(tmp); };
	const auto printperc = [](double d) {char tmp[64]; sprintf_s(tmp, "%04.1lf%%", 100.0 * (d < 0.0 ? 0.0 : (d > 1.0 ? 1.0 : d))); return std::string(tmp); };

	std::cout << "Loading file..." << std::endl;
	const auto things = readfile(inuse);
	if (things.empty()) return 1;
	std::cout << "Translating stuff..." << std::endl;
	const auto things2 = get_preds_steps(things, minchuva);
	std::cout << "Got " << things2.size() << " entities" << std::endl;

	// ENABLE_TEST ainda não foi terminado
#ifdef ENABLE_TEST
	std::cout << "\n\nTesting algorithm..." << std::endl;

	std::cout << "Testing all possibilities... (" << things.size() << ") asynchronously..." << std::endl;

	struct __a {
		//long double score_totally = std::numeric_limits<long double>::lowest();
		unsigned long long score_acerto = 0, score_erro = std::numeric_limits<unsigned long long>::max();
		//double selected_best[3]{}; // a,b,c
		double konst[num_ks]{};
		double step = 0.0;
		unsigned long long progress = 0;
		size_t count_tasks = 0;
	} results[thr_count];

	{
		Paralleler pll;
		//std::atomic<unsigned long long> hugecounter = 0;
		const auto totalcountereach = powl((thr_count * thr_steps), (num_ks + 1)) * 1.0 / thr_count;

		for (size_t pa = 0; pa < thr_count; ++pa) {
			pll.summon([pa, &results, &printperc, &things]() {
				auto& r = results[pa];

				double temp_konsts_mempos[num_ks]{};
				recursive_loop_of({ pa, thr_count }, 0.0, 1.0, thr_precision, temp_konsts_mempos, [&](const double(&Ks)[num_ks]) {
					++r.progress;
					r.step = Ks[num_ks - 1];

					unsigned long long ac = 0, er = 0;
					for (size_t px = 0; px < things.size(); ++px) {
						pairing8 pr;
						const bool shouldbe = validity_check_get(things, px, pr);
						const double got = chances_of(pr, Ks);

						if (got > 0.5 && !shouldbe || got <= 0.5 && shouldbe) ++er;
						else ++ac;
					}

					if (ac > r.score_acerto && er < r.score_erro) {
						r.score_acerto = ac;
						r.score_erro = er;
						for (size_t p = 0; p < num_ks; ++p) r.konst[p] = Ks[p];
					}
				});
				//
				//const double slicenum = 1.0 / thr_count;
				//for (double _Ka = 0.0; _Ka < slicenum; _Ka += thr_precision) {
				//	const double Ka = (static_cast<double>(pa) / thr_count) + _Ka;
				//
				//	const double ext_perc = (static_cast<double>(Ka - (static_cast<double>(pa) / thr_count)) / thr_count);
				//	//const double ext_relation = 1.0 / thr_count;
				//	const double KaPerc = (_Ka * 1.0 / slicenum);
				//
				//	for (double Kb = 0.0; Kb <= 1.0; Kb += thr_precision) {
				//		for (double Kc = 0.0; Kc <= 1.0; Kc += thr_precision) {
				//
				//			//r.progress = ext_perc + ext_relation * (Kb + Kc * thr_precision);
				//
				//			r.progress = KaPerc + ((Kb + (Kc * thr_precision)) * slicenum);
				//
				//			//if (r.count_tasks % 1000 == 0) std::cout << printperc(Ka) << "; " << printperc(Kb) << "; " << printperc(Kc) << " [" << r.count_tasks << "] \r";
				//
				//			//long double score_curr = 0;
				//
				//			unsigned long long temp_acert = 0, temp_err = 0;
				//
				//			for (size_t px = 0; px < things.size(); ++px) {
				//				++r.count_tasks;
				//				pairing8 pr;
				//				const bool shouldbe = validity_check_get(things, px, pr);
				//				const double got = chances_of(pr, Ka, Kb, Kc);
				//
				//				if (got >= 0.6 && !shouldbe) {
				//					++temp_err;
				//					//score_curr -= 2.0 * (got - 0.5);
				//				}
				//				else if (got <= 0.6 && shouldbe) {
				//					++temp_err;
				//					//score_curr -= 4.0 * (got);
				//				}
				//				else {
				//					++temp_acert;
				//					//score_curr += 1.5 * got;
				//				}
				//				//score_totally += ( || (got <= 0.5 && shouldbe))
				//			}
				//
				//			if (temp_acert > r.score_acerto && temp_err < r.score_erro) {
				//				r.score_acerto = temp_acert;
				//				r.score_erro = temp_err;
				//				r.selected_best[0] = Ka;
				//				r.selected_best[1] = Kb;
				//				r.selected_best[2] = Kc;
				//			}
				//		}
				//	}
				//}
			});
		}

		bool kii = false;
		std::thread async_print([&] {
			while (!kii) {
				for (const auto& i : results) {
					std::cout << printperc(i.progress * 100.0 / totalcountereach) << "[" << printdbs(i.step) << "]" << " ";
				}
				std::cout << "\r";
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
			}
		});

		pll.wait_all();
		kii = true;
		async_print.join();
	}

	long double score_totally = 0.0;
	double selected_best[num_ks]{}; // a,b,c

	for (const auto& i : results) {
		if (const long double _t = (i.score_acerto * 1.0 / (i.score_erro + i.score_acerto)); _t > score_totally) {
			score_totally = _t;
		//if (i.score_totally > score_totally) {
		//	score_totally = i.score_totally;
			for (size_t p = 0; p < num_ks; ++p) selected_best[p] = i.konst[p];
		}
	}

	//for (double Ka = 0.0; Ka <= 1.0; Ka += 0.01) {
	//	for (double Kb = 0.0; Kb <= 1.0; Kb += 0.01) {
	//		for (double Kc = 0.0; Kc <= 1.0; Kc += 0.01) {
	//
	//			if (count_tasks % 1000 == 0) std::cout << printperc(Ka) << "; " << printperc(Kb) << "; " << printperc(Kc) << " [" << count_tasks << "] \r";
	//
	//			long double score_curr = 0.0;
	//
	//			for (size_t px = 0; px < things.size(); ++px) {
	//				++count_tasks;
	//				pairing8 pr;
	//				const bool shouldbe = validity_check_get(things, px, pr);
	//				const double got = chances_of(pr, Ka, Kb, Kc);
	//
	//				if (got >= 0.6 && !shouldbe) {
	//					score_curr -= 4.0 * (got - 0.5);
	//				}
	//				else if (got <= 0.6 && shouldbe) {
	//					score_curr -= 10.0 * (got);
	//				}
	//				else {
	//					score_curr += 3.0 * got;
	//				}
	//				//score_totally += ( || (got <= 0.5 && shouldbe))
	//			}
	//
	//			if (score_curr > score_totally) {
	//				score_totally = score_curr;
	//				selected_best[0] = Ka;
	//				selected_best[1] = Kb;
	//				selected_best[2] = Kc;
	//			}
	//		}
	//	}
	//}

	std::cout << "\nAlgorithm for all stuff got best score: " << printdb(100.0 * score_totally) << "." << std::endl;
	for (size_t p = 0; p < num_ks; ++p) std::cout << "K" << (char)('a' + p) << ": " << printdb(selected_best[p]) << std::endl;


	std::cout << "\nTesting the perfect ones." << std::endl;

	{
		long double score_correcte = 0.0;
		for (const auto& ea : things2) {
			const double got = chances_of(ea, selected_best);
			score_correcte += static_cast<long double>((got - 0.7) * (got < 0.7 ? 5.0 : 1.0)); // penalty 5x if bad, else ++
		}

		std::cout << "Algorithm got score of correctness: " << printdb(score_correcte) << " of " << things2.size() << " entities" << std::endl;
	}

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
			al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgb(255, 255, 255), thicc_avg);
			al_draw_filled_circle(x1, bmpsiz - b4, thicc_avg * 0.5f, al_map_rgb(255, 255, 255));
			al_draw_filled_circle(x2, bmpsiz - af, thicc_avg * 0.5f, al_map_rgb(255, 255, 255));
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
			al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgb(255, 255, 255), thicc_avg);
			al_draw_filled_circle(x1, bmpsiz - b4, thicc_avg * 0.5f, al_map_rgb(255, 255, 255));
			al_draw_filled_circle(x2, bmpsiz - af, thicc_avg * 0.5f, al_map_rgb(255, 255, 255));
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
			al_draw_line(x1, bmpsiz - b4, x2, bmpsiz - af, al_map_rgb(255, 255, 255), thicc_avg);
			al_draw_filled_circle(x1, bmpsiz - b4, thicc_avg * 0.5f, al_map_rgb(255, 255, 255));
			al_draw_filled_circle(x2, bmpsiz - af, thicc_avg * 0.5f, al_map_rgb(255, 255, 255));
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