#pragma once

// note to myself: each bit = 1/0.
// RIGHT -> LEFT, TOP -> BOTTOM!

// 8x8
static unsigned char u8g_ok[] = {
  0b11000000,
  0b11000000,
  0b11100000,
  0b01100000,
  0b01110011,
  0b00110111,
  0b00011110,
  0b00001100
};

// 8x8
static unsigned char u8g_fail[] = {
  0b11000011,
  0b11100111,
  0b01111110,
  0b00111100,
  0b00111100,
  0b01111110,
  0b11100111,
  0b11000011
};

static unsigned char u8g_charge[] = {
  0b00011111,
  0b00111110,
  0b01111100,
  0b11111111,
  0b00011110,
  0b00111100,
  0b01111000,
  0b11100000,
};
