#include "terminal.h"

const char blank_line[] = "                                                            ";
char linebuffer[] = "                                                            ";
uint8_t *const linebuffer_const = (uint8_t*)linebuffer;

History::History() {
  lines_per_screen = 34;
  scrollback_length = 200;
  line_count = 1;
  cursor_index = 0;
  last_line_address = 0;
  set_scrollbar_handle_size();
}

void History::update_scrollbar_position(uint16_t new_position) {
  scrollbar_position = new_position;
  if (scrollbar_position < scrollbar_size_half)
    scrollbar_position = scrollbar_size_half;
  if (scrollbar_position > 65535 - scrollbar_size_half)
    scrollbar_position = 65535 - scrollbar_size_half;
  scrollbar_position_percent = ((float)scrollbar_position - (float)scrollbar_size_half) / (65535.0 - (float)scrollbar_size);
  scrollbar_position_percent *= 100;
  scrollbar_position_percent = floor(scrollbar_position_percent);
  scrollbar_position_percent /= 100;
  scrollbar_position_percent = 1.0 - scrollbar_position_percent;
  if (line_count <= lines_per_screen) {
    scroll_offset = 0;
  }
  else {
    scroll_offset = (uint16_t) (scrollbar_position_percent * ((float)line_count - lines_per_screen));
  }
}

void History::upload_to_graphics_ram() {
  GD.cmd_memwrite(last_line_address*CHARACTERS_PER_LINE, CHARACTERS_PER_LINE);
  GD.copy(linebuffer_const, CHARACTERS_PER_LINE);

}

void History::set_scrollbar_handle_size() {
  lines_per_screen_percent = ((float) lines_per_screen) / ((float) line_count);
  if (lines_per_screen_percent > 1.0)
    lines_per_screen_percent = 1.0;
  scrollbar_size = (uint16_t) floor(lines_per_screen_percent * 65535.0);
  scrollbar_size_half = (uint16_t) floor(lines_per_screen_percent * 0.5 * 65535.0);
  update_scrollbar_position(65535);
}

int32_t unread_count;

void History::new_line() {
  // copy linebuffer to FT810 RAM
  upload_to_graphics_ram();
  cursor_index = 0;
  line_count++;
  if (line_count >= scrollback_length)
    line_count = scrollback_length;
  last_line_address++;
  if (last_line_address > scrollback_length)
    last_line_address = 0;
  // erase current line
  strncpy(linebuffer, blank_line, CHARACTERS_PER_LINE);
  // sprintf(linebuffer, "%-3d", unread_count);
  set_scrollbar_handle_size();
}

void History::append_string(const char* str) {
  for(int i=0; i<strlen(str); i++) {
    append_character(str[i]);
  }
  // for(char& c : str) {
  //   // append_character(c);
  // }
  // append_character((char) 13);
}

uint8_t History::append_character(char newchar) {
  if (cursor_index >= CHARACTERS_PER_LINE || newchar == 13 || newchar == 10) {
    new_line();
    return LINE_FULL;
  }
  else {
    linebuffer[cursor_index++] = newchar;
    return true;
  }
}

void History::draw() {
  GD.BitmapSize(NEAREST, BORDER, BORDER, 480, 8);
  GD.BitmapLayout(TEXT8X8, CHARACTERS_PER_LINE, 1);
  GD.Begin(BITMAPS);
  GD.ColorRGB(WHITE);

  uint16_t current_line_address = last_line_address;
  if (scroll_offset > 0)
    current_line_address = (current_line_address + (scrollback_length-scroll_offset)) % scrollback_length;

  int16_t ycoord;
  uint16_t min_lines = line_count;
  if (line_count > lines_per_screen)
    min_lines = lines_per_screen;

  for (int i=0; i<min_lines; i++) {
    ycoord = GD.h - LINE_PIXEL_HEIGHT - (LINE_PIXEL_HEIGHT * i);
    GD.BitmapSource( current_line_address * CHARACTERS_PER_LINE );
    GD.Vertex2ii(0, ycoord);
    current_line_address = (current_line_address + (scrollback_length-1)) % scrollback_length;
  }

  GD.ColorA(64); // alpha to 64/256
  GD.cmd_bgcolor(VALHALLA);
  GD.cmd_fgcolor(LIGHT_STEEL_BLUE);

  GD.Tag(TAG_SCROLLBAR);
  GD.cmd_scrollbar(GD.w - SCROLLBAR_WIDTH, SCROLLBAR_HALF_WIDTH, SCROLLBAR_WIDTH, GD.h-SCROLLBAR_WIDTH, OPT_FLAT, scrollbar_position - scrollbar_size_half, scrollbar_size, 65535);
      GD.cmd_track(GD.w - SCROLLBAR_WIDTH, SCROLLBAR_HALF_WIDTH, SCROLLBAR_WIDTH, GD.h-SCROLLBAR_WIDTH, TAG_SCROLLBAR);
}
