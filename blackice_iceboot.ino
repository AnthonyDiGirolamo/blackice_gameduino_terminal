#include <MyStorm.h>
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

const char blank_line[] = "                                                            ";
char linebuffer[] = "                                                            ";
uint8_t *const linebuffer_const = (uint8_t*)linebuffer;

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

  void append_character(char newchar);
  void update_scrollbar_position(uint16_t new_position);
  void set_scrollbar_handle_size();
  void make_new_line();
  void upload_to_graphics_ram();
  void draw();
};

History::History() {
  lines_per_screen = 34;
  scrollback_length = 100;
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
  lines_per_screen_percent = min(1.0, ((float) lines_per_screen) / ((float) line_count));
  scrollbar_size = (uint16_t) floor(lines_per_screen_percent * 65535.0);
  scrollbar_size_half = (uint16_t) floor(lines_per_screen_percent * 0.5 * 65535.0);
  update_scrollbar_position(65535);
}

void History::make_new_line() {
  cursor_index = 0;
  line_count++;
  if (line_count >= scrollback_length)
    line_count = scrollback_length;
  last_line_address++;
  if (last_line_address > scrollback_length)
    last_line_address = 0;
  // erase current line
  strncpy(linebuffer, blank_line, CHARACTERS_PER_LINE);
  set_scrollbar_handle_size();
}

void History::append_character(char newchar) {
  if (cursor_index >= CHARACTERS_PER_LINE || newchar == 13 || newchar == 10) {
    make_new_line();
  }
  else {
    linebuffer[cursor_index++] = newchar;
  }
}

void History::draw() {
  GD.BitmapSize(NEAREST, BORDER, BORDER, 480, 8);
  GD.BitmapLayout(TEXT8X8, CHARACTERS_PER_LINE, 1);
  GD.Begin(BITMAPS);
  GD.ColorRGB(WHITE);

  uint16_t current_line_address = last_line_address;
  // TODO add scroll offset
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

  GD.ColorA(64); // 64/256
  GD.cmd_bgcolor(VALHALLA);
  GD.cmd_fgcolor(LIGHT_STEEL_BLUE);

  GD.Tag(TAG_SCROLLBAR);
  GD.cmd_scrollbar(GD.w - SCROLLBAR_WIDTH, SCROLLBAR_HALF_WIDTH, SCROLLBAR_WIDTH, GD.h-SCROLLBAR_WIDTH, OPT_FLAT, scrollbar_position - scrollbar_size_half, scrollbar_size, 65535);
      GD.cmd_track(GD.w - SCROLLBAR_WIDTH, SCROLLBAR_HALF_WIDTH, SCROLLBAR_WIDTH, GD.h-SCROLLBAR_WIDTH, TAG_SCROLLBAR);
}


char numbers[256];
char get_char() {
  return numbers[GD.random(256)];
}

uint32_t i, t;

History lines;

void setup() {
  for(i=0; i<256; i++)
    numbers[i]=i;

  GD.begin();
  lines.upload_to_graphics_ram();

  Serial.begin(115200);
  Serial1.begin(115200);

  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, 1);
  // myStorm.FPGAConfigure((const byte*)0x0801F000, 135100);
  // digitalWrite(LED_BUILTIN, 0);
}

void loop() {
  char newchar;
  // for(i=0; i<GD.random(10); i++) {
  if (lines.line_count < 100) {
    for(i=0; i<40; i++) {
      newchar = get_char();
      lines.append_character(newchar);
      sprintf(linebuffer, "%-3u", lines.line_count);
    }
  }
  else {
    sprintf(linebuffer, "%u  %u  %u  %f  %u  ", lines.scrollbar_size, lines.scrollbar_size_half, lines.scrollbar_position, lines.scrollbar_position_percent, lines.scroll_offset);
  }

  GD.get_inputs();
  switch (GD.inputs.track_tag & 0xff) {
  case TAG_SCROLLBAR:
    lines.update_scrollbar_position(GD.inputs.track_val);
  }

  lines.upload_to_graphics_ram();

  GD.Clear();

  lines.draw();

  // GD.BitmapSource(60);
  // GD.Vertex2ii(0, 9);
  GD.swap();

  // Serial.print(analogRead(3)); // horiz
  // Serial.print("  ");
  // Serial.print(analogRead(5)); // vert
  // Serial.println("");

  // if (Serial.available()) {
  //   digitalWrite(LED_BUILTIN, 1);
  //   if (myStorm.FPGAConfigure(Serial)) {
  //     while (Serial.available())
  //       Serial.read();
  //     Serial.println("FPGAConfigure done");
  //   }
  //   digitalWrite(LED_BUILTIN, 0);
  // }
  // else if (Serial1.available() > 0) {
  //   Serial.write(Serial1.read());
  // }

}

