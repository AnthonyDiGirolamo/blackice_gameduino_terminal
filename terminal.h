#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <Arduino.h>
#include <SPI.h>
#include <GD2.h>

// DB32 Pallete
// https://github.com/geoffb/dawnbringer-palettes

#define BLACK            0x000000  // #000000
#define VALHALLA         0x222034  // #222034
#define LOULOU           0x45283c  // #45283c
#define OILED_CEDAR      0x663931  // #663931
#define ROPE             0x8f563b  // #8f563b
#define TAHITI_GOLD      0xdf7126  // #df7126
#define TWINE            0xd9a066  // #d9a066
#define PANCHO           0xeec39a  // #eec39a

#define GOLDEN_FIZZ      0xfbf236  // #fbf236
#define ATLANTIS         0x99e550  // #99e550
#define CHRISTI          0x6abe30  // #6abe30
#define ELF_GREEN        0x37946e  // #37946e
#define DELL             0x4b692f  // #4b692f
#define VERDIGRIS        0x524b24  // #524b24
#define OPAL             0x323c39  // #323c39
#define DEEP_KOAMARU     0x3f3f74  // #3f3f74

#define VENICE_BLUE      0x306082  // #306082
#define ROYAL_BLUE       0x5b6ee1  // #5b6ee1
#define CORNFLOWER       0x639bff  // #639bff
#define VIKING           0x5fcde4  // #5fcde4
#define LIGHT_STEEL_BLUE 0xcbdbfc  // #cbdbfc
#define WHITE            0xffffff  // #ffffff
#define HEATHER          0x9badb7  // #9badb7
#define TOPAZ            0x847e87  // #847e87

#define DIM_GRAY         0x696a6a  // #696a6a
#define SMOKEY_ASH       0x595652  // #595652
#define CLAIRVOYANT      0x76428a  // #76428a
#define BROWN            0xac3232  // #ac3232
#define MANDY            0xd95763  // #d95763
#define PLUM             0xd77bba  // #d77bba
#define RAINFOREST       0x8f974a  // #8f974a
#define STINGER          0x8a6f30  // #8a6f30

#define LINE_PIXEL_HEIGHT 8
#define CHARACTERS_PER_LINE 60
#define SCROLLBAR_WIDTH 20
#define SCROLLBAR_HALF_WIDTH 10

#define TAG_SCROLLBAR 201
#define TAG_BUTTON1   202
#define TAG_BUTTON2   203

#define LINE_FULL 0
#define CHAR_READ 1

class History {
public:
  History();
  uint16_t cursor_index;
  uint16_t line_count;
  uint16_t last_line_address;
  uint16_t lines_per_screen;
  uint16_t scrollback_length;
  float lines_per_screen_percent;
  uint16_t scrollbar_size;
  uint16_t scrollbar_size_half;
  uint16_t scrollbar_position;
  float scrollbar_position_percent;
  uint16_t scroll_offset;

  void append_string(const char* str);
  uint8_t append_character(char newchar);
  void update_scrollbar_position(uint16_t new_position);
  void set_scrollbar_handle_size();
  void new_line();
  void upload_to_graphics_ram();
  void draw();
};

#endif
