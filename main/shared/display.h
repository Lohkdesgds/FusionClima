#pragma once

extern "C" {
    #include "u8g2.h"

    #include "driver/gpio.h"
    #include "driver/spi_master.h"
    #include "driver/i2c.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_log.h"
    #include "sdkconfig.h"
    #include <string.h>

    typedef struct {
        gpio_num_t clk;
        gpio_num_t mosi;
        gpio_num_t sda;
        gpio_num_t scl;
        gpio_num_t cs;
        gpio_num_t reset;
        gpio_num_t dc;
    } u8g2_esp32_hal_t;
}

class Display {
    u8g2_t m_dsp{};
public:
    void setup(const gpio_num_t sda, const gpio_num_t scl, const gpio_num_t rst);

    void SendBuffer();
    void ClearBuffer();
    void FirstPage();
    uint8_t NextPage();
    void UpdateDisplayArea(uint8_t  tx, uint8_t ty, uint8_t tw, uint8_t th);
    void UpdateDisplay();
    void DrawHVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir);
    void DrawHLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len);
    void DrawVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len);
    void DrawPixel(u8g2_uint_t x, u8g2_uint_t y);
    void SetDrawColor(uint8_t color);
    void SetBitmapMode(uint8_t is_transparent);
    void DrawHorizontalBitmap(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, const uint8_t *b);
    void DrawBitmap(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t cnt, u8g2_uint_t h, const uint8_t *bitmap);
    void DrawXBM(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap);
    void DrawXBMP(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap);
    void DrawCircle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option);
    void DrawDisc(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option);
    void DrawEllipse(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option);
    void DrawFilledEllipse(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t option);
    void DrawLine(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2);
    void DrawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);
    void DrawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);
    void DrawRBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r);
    void DrawRFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r);
    void DrawButtonFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t flags, u8g2_uint_t text_width, u8g2_uint_t padding_h, u8g2_uint_t padding_v);
    void DrawButtonUTF8(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t flags, u8g2_uint_t width, u8g2_uint_t padding_h, u8g2_uint_t padding_v, const char *text);
    void ClearPolygonXY(void);
    void AddPolygonXY(int16_t x, int16_t y);
    void DrawPolygon();
    void DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    uint8_t GetKerning(u8g2_kerning_t *kerning, uint16_t e1, uint16_t e2);
    uint8_t GetKerningByTable(const uint16_t *kt, uint16_t e1, uint16_t e2);
    void SetFont(const uint8_t  *font);
    void SetFontMode(uint8_t is_transparent);
    uint8_t IsGlyph(uint16_t requested_encoding);
    int8_t GetGlyphWidth(uint16_t requested_encoding);
    u8g2_uint_t DrawGlyph(u8g2_uint_t x, u8g2_uint_t y, uint16_t encoding);
    u8g2_uint_t DrawGlyphX2(u8g2_uint_t x, u8g2_uint_t y, uint16_t encoding);
    int8_t GetStrX(const char *s);	
    void SetFontDirection(uint8_t dir);
    u8g2_uint_t DrawStr(u8g2_uint_t x, u8g2_uint_t y, const char *str);
    u8g2_uint_t DrawStrX2(u8g2_uint_t x, u8g2_uint_t y, const char *str);
    u8g2_uint_t DrawUTF8(u8g2_uint_t x, u8g2_uint_t y, const char *str);
    u8g2_uint_t DrawUTF8X2(u8g2_uint_t x, u8g2_uint_t y, const char *str);
    u8g2_uint_t DrawExtendedUTF8(u8g2_uint_t x, u8g2_uint_t y, uint8_t to_left, u8g2_kerning_t *kerning, const char *str);
    u8g2_uint_t DrawExtUTF8(u8g2_uint_t x, u8g2_uint_t y, uint8_t to_left, const uint16_t *kerning_table, const char *str);
    uint8_t IsAllValidUTF8(const char *str);
    u8g2_uint_t GetStrWidth(const char *s);
    u8g2_uint_t GetUTF8Width(const char *str);
    void SetFontPosBaseline();
    void SetFontPosBottom();
    void SetFontPosTop();
    void SetFontPosCenter();
    void SetFontRefHeightText();
    void SetFontRefHeightExtendedText();
    void SetFontRefHeightAll();
    void DrawUTF8Line(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, const char *s, uint8_t border_size, uint8_t is_invert);
    u8g2_uint_t DrawUTF8Lines(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t line_height, const char *s);
    uint8_t UserInterfaceSelectionList(const char *title, uint8_t start_pos, const char *sl);

};

inline Display u8g2;