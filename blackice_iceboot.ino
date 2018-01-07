#include <MyStorm.h>
#include <SPI.h>
#include <GD2.h>

#define BLACK            0x000000
#define VALHALLA         0x222034
#define LOULOU           0x45283c
#define OILED_CEDAR      0x663931
#define ROPE             0x8f563b
#define TAHITI_GOLD      0xdf7126
#define TWINE            0xd9a066
#define PANCHO           0xeec39a

#define GOLDEN_FIZZ      0xfbf236
#define ATLANTIS         0x99e550
#define CHRISTI          0x6abe30
#define ELF_GREEN        0x37946e
#define DELL             0x4b692f
#define VERDIGRIS        0x524b24
#define OPAL             0x323c39

#define DEEP_KOAMARU     0x3f3f74
#define VENICE_BLUE      0x306082
#define ROYAL_BLUE       0x5b6ee1
#define CORNFLOWER       0x639bff
#define VIKING           0x5fcde4
#define LIGHT_STEEL_BLUE 0xcbdbfc
#define WHITE            0xffffff
#define HEATHER          0x9badb7

#define TOPAZ            0x847e87
#define DIM_GRAY         0x696a6a
#define SMOKEY_ASH       0x595652
#define CLAIRVOYANT      0x76428a
#define BROWN            0xac3232
#define MANDY            0xd95763
#define PLUM             0xd77bba
#define RAINFOREST       0x8f974a
#define STINGER          0x8a6f30

#define LINE_PIXEL_HEIGHT 8
#define CHARACTERS_PER_LINE 60
#define LINES_PER_SCREEN 34
#define SCROLLBACK_LENGTH 100

const char blank_line[] = "                                                            ";
char linebuffer[] = "                                                            ";
uint8_t *const linebuffer_const = (uint8_t*)linebuffer;

class History {
public:
  History();
  uint16_t cursor_index;
  uint16_t line_count;
  uint16_t last_line_address;
  void append_character(char newchar);
  void make_new_line();
  void upload_to_graphics_ram();
  void draw();
};

History::History() {
  cursor_index = 0;
  line_count = 1;
  last_line_address = 0;
}

void History::upload_to_graphics_ram() {
  GD.cmd_memwrite(last_line_address*CHARACTERS_PER_LINE, CHARACTERS_PER_LINE);
  GD.copy(linebuffer_const, CHARACTERS_PER_LINE);
}

void History::append_character(char newchar) {
  if (cursor_index >= CHARACTERS_PER_LINE || newchar == 13 || newchar == 10) {
    cursor_index = 0;
    line_count++;
    if (line_count >= SCROLLBACK_LENGTH)
      line_count = SCROLLBACK_LENGTH;
    last_line_address++;
    if (last_line_address > SCROLLBACK_LENGTH)
      last_line_address = 0;
    // erase current line
    strncpy(linebuffer, blank_line, CHARACTERS_PER_LINE);
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

  int16_t ycoord;
  uint16_t min_lines = line_count;
  if (line_count > LINES_PER_SCREEN)
    min_lines = LINES_PER_SCREEN;

  for (int i=0; i<min_lines; i++) {
    ycoord = GD.h - LINE_PIXEL_HEIGHT - (LINE_PIXEL_HEIGHT * i);
    GD.BitmapSource( current_line_address * CHARACTERS_PER_LINE );
    GD.Vertex2ii(0, ycoord);
    current_line_address = (current_line_address + (SCROLLBACK_LENGTH-1)) % SCROLLBACK_LENGTH;
  }
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
  if (lines.line_count < SCROLLBACK_LENGTH) {
    for(i=0; i<GD.random(10); i++) {
      newchar = get_char();
      lines.append_character(newchar);
    }
    lines.upload_to_graphics_ram();
  }

  // t = millis();
  // sprintf(linebuffer, "%-60d", t);

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

