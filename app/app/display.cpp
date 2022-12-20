#include "display.h"

void Display::create_and_load()
{
	if (!al_is_system_installed()) al_init();
	if (!al_is_image_addon_initialized()) al_init_image_addon();
	if (!al_is_primitives_addon_initialized()) al_init_primitives_addon();

	destroy();

	al_set_new_window_title("Loading...");
	if (!(m_dsp = al_create_display(display_size[0], display_size[1]))) throw 0;
	_update_title();

	m_bkg_fp_mem = std::unique_ptr<char[]>(new char[std::size(FileSpace::bkg)]);
	if (!(m_bkg_fp = al_open_memfile(m_bkg_fp_mem.get(), std::size(FileSpace::bkg), "rw"))) throw 1;
	al_fwrite(m_bkg_fp, FileSpace::bkg, std::size(FileSpace::bkg));
	al_fflush(m_bkg_fp);
	al_fseek(m_bkg_fp, 0, ALLEGRO_SEEK_SET);
	if (!(m_bkg = al_load_bitmap_f(m_bkg_fp, ".JPG"))) throw 2;
}

void Display::destroy()
{
	if (m_bkg) al_destroy_bitmap(m_bkg);
	m_bkg = nullptr;
	if (m_bkg_fp) al_fclose(m_bkg_fp);
	m_bkg_fp = nullptr;
	m_bkg_fp_mem.reset();
	if (m_dsp) al_destroy_display(m_dsp);
	m_dsp = nullptr;
}

DisplayStatus& Display::get()
{
	return m_stat;
}

void Display::draw()
{
	al_draw_bitmap(m_bkg, 0, 0, 0);

	if (m_stat.connected) al_draw_filled_circle(al_get_bitmap_width(m_bkg) * 0.5f, 260.0f * display_orig_size_scale[1], 15.0f, al_map_rgb(0, 255, 0));
	else				  al_draw_filled_circle(al_get_bitmap_width(m_bkg) * 0.5f, 260.0f * display_orig_size_scale[1], 15.0f, al_map_rgb(255, 0, 0));

	if (m_stat.bar_complete != 1.0f) {
		al_draw_filled_rectangle(
			al_get_bitmap_width(m_bkg) - 20.0f * display_orig_size_scale[0], 
			90.0f * display_orig_size_scale[1], 
			20.0f * display_orig_size_scale[0] + (al_get_bitmap_width(m_bkg) - 40.0f * display_orig_size_scale[0]) * m_stat.bar_complete,
			120.0f * display_orig_size_scale[1], al_map_rgb(16, 16, 16));
	}

	al_flip_display();
}

Display::operator bool() const
{
	return m_dsp;
}

bool Display::valid() const
{
	return m_dsp;
}

void Display::set_title(const std::string& titl)
{
	m_title = titl;
	_update_title();
}

void Display::set_end_title(const std::string& dsc)
{
	m_desc = dsc;
	_update_title();
}

ALLEGRO_EVENT_SOURCE* Display::get_event() const
{
	return m_dsp ? al_get_display_event_source(m_dsp) : nullptr;
}

void Display::_update_title()
{
	if (m_dsp) al_set_window_title(m_dsp, (m_title + (m_desc.empty() ? "" : " - " + m_desc)).c_str());
}
