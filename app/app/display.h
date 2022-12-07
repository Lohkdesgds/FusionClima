#pragma once
#define WIN32_LEAN_AND_MEAN

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_memfile.h>

#include "image_bg.h"

#include <memory>
#include <string>

constexpr int display_size[] = {700, 400};
constexpr float display_orig_size_scale[] = { display_size[0] * 1.0f / 700.0f, display_size[1] * 1.0f / 400.0f};

struct DisplayStatus {
	bool connected = false;
	float bar_complete = 0.5f;
};

class Display {
public:
	void create_and_load();
	void destroy();

	DisplayStatus& get();
	void draw();

	operator bool() const;
	bool valid() const;

	void set_title(const std::string&);
	void set_end_title(const std::string&);

	ALLEGRO_EVENT_SOURCE* get_event() const;
private:
	void _update_title();

	ALLEGRO_DISPLAY* m_dsp = nullptr;
	ALLEGRO_BITMAP* m_bkg = nullptr;
	ALLEGRO_FILE* m_bkg_fp = nullptr;
	std::unique_ptr<char[]> m_bkg_fp_mem;
	std::string m_title = "Fusion Clima V1.0", m_desc;

	DisplayStatus m_stat;
};