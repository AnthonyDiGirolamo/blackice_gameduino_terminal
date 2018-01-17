#include <FS.h>
#include <MyStorm.h>
#include <SPI.h>
#include <GD2.h>
#include "circular_buffer.h"
#include "terminal.h"

#define TAG_BUTTON_MACRO 202
#define TAG_BUTTON_RETURN 203
#define TAG_BUTTON_BACKSPACE 204
#define TAG_BUTTON_BELL 205
#define TAG_BUTTON_FONT 206

#define BUTTON_REPEAT_RATE 250
uint32_t last_button_push = millis();

Terminal terminal;

// Additional serial_buffer to hold bursts of data.
circular_buffer<uint8_t> serial_buffer(2048);

volatile uint32_t last_serial_recieve_time = millis();

// // debugging vars for monitoring serial_buffer size
// volatile int32_t last_pending_chars = 1;
// volatile int32_t pending_chars = 0;

void buffer_serial_data() {
  __disable_irq(); // don't interrupt inside this interrupt
  while (Serial1.available() > 0) {
    serial_buffer.put(Serial1.read());
    // pending_chars++;
  }
  last_serial_recieve_time = millis();
  __enable_irq(); // re-enable interrupts
}

void setup() {
  GD.begin(); // init gameduino

  // Serial to PC
  Serial.begin(115200);

  // Serial to FPGA
  // TXD 88 ( P2 PMOD 1/2 )
  // RXD 85 ( P3 PMOD 1/2 )
  Serial1.begin(115200);

  // When new data comes in on Serial1, run buffer_serial_data()
  Serial1.onReceive(buffer_serial_data);

  // Configure FPGA from the STM32 flash
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, 1);
  // myStorm.FPGAConfigure((const byte*)0x0801F000, 135100);
  // digitalWrite(LED_BUILTIN, 0);

  // send a string to the terminal
  terminal.append_string("Color Test\n\n");

  // TextVGA test
  char cnum[] = "   ";
  uint8_t fg = 0, bg = 0;
  sprintf(cnum, "%-3u", fg);
  while (1) {
    GD.Clear();
    if (millis() > last_serial_recieve_time + 100) {
      terminal.foreground_color = fg;
      terminal.background_color = 0;

      // if (fg % 10 == 0) {
      // bg = (bg+1)%16;
      terminal.new_line();
      terminal.append_string("fg ");
      sprintf(cnum, "%-3u", fg);
      terminal.append_string(cnum);

      terminal.append_string("bg ");

      for (bg = 0; bg<16; bg++) {
        terminal.background_color = bg;
        sprintf(cnum, "%-3u", bg);
        terminal.append_string(cnum);
      }

      terminal.upload_to_graphics_ram();
      fg = (fg+1)%256;
      last_serial_recieve_time = millis();
    }
    terminal.draw();
    GD.swap();
    if (fg > 15)
      break;
  }
  terminal.new_line();
  terminal.foreground_color = 15;
  terminal.background_color = 0;

  // Check the SD card for bin files to configure the FPGA with
  // if 'default.bin' exists, use that
  if (DOSFS.begin() && DOSFS.check()) {
    terminal.append_string("SDCard Files:\n\n");
    Dir dir = DOSFS.openDir("/");
    do {
      terminal.append_string(dir.fileName().c_str());
      terminal.new_line();
    } while (dir.next());

    if (DOSFS.exists("default.bin")) {
      terminal.append_string("\nFound 'default.bin'\nConfiguring FPGA...");
      GD.Clear();
      terminal.draw();
      GD.swap();

      File file = DOSFS.open("default.bin", "r");
      if (file) {
        // use the file as an Arduino Stream to configure the FPGA
        digitalWrite(LED_BUILTIN, 1);
        myStorm.FPGAConfigure(file);
        digitalWrite(LED_BUILTIN, 0);
        file.close();
        terminal.append_string("Done\n\n");
      }
    }

    DOSFS.end();
  }
  else {
    terminal.append_string("No SDCard found.\n\n");
  }

  terminal.upload_to_graphics_ram();
  GD.Clear();
  terminal.draw();
  GD.swap();

}

void loop() {
  uint8_t prevkey;
  uint8_t key;
  uint8_t append_character_result;

  while (1) {

    // if we aren't receiving any new data
    if (millis() > last_serial_recieve_time + 100) {

      while (!serial_buffer.empty()) {
        // upload new data to the GPU ram
        append_character_result = terminal.append_character(serial_buffer.get());
        // pending_chars--;
        // if we just completed a single line
        if (append_character_result == LINE_FULL) {
          // break out of the loop so the screen can update
          break;
        }
      }

      // if everything has been processed
      if (serial_buffer.empty()) {
        // upload the current line to the GPU
        terminal.upload_to_graphics_ram();
      }

      GD.get_inputs();

      // scrollbar
      switch (GD.inputs.track_tag & 0xff) {
      case TAG_SCROLLBAR:
        terminal.update_scrollbar_position(GD.inputs.track_val);
        break;
      }

      // buttons
      switch (GD.inputs.track_tag) {
      case TAG_BUTTON_MACRO:
        if (millis() > last_button_push + BUTTON_REPEAT_RATE) {
          last_button_push = millis();
          Serial1.write("words\n");
        }
        break;
      case TAG_BUTTON_FONT:
        if (millis() > last_button_push + BUTTON_REPEAT_RATE) {
          if (terminal.current_font == TEXT8X8)
            terminal.set_font_vga();
          else
            terminal.set_font_8x8();
        }
        break;
      case TAG_BUTTON_BELL:
        if (millis() > last_button_push + BUTTON_REPEAT_RATE) {
          terminal.ring_bell();
        }
        break;
      case TAG_BUTTON_BACKSPACE:
        if (millis() > last_button_push + BUTTON_REPEAT_RATE) {
          last_button_push = millis();
          Serial1.write(KEY_BACKSPACE);
        }
        break;
      case TAG_BUTTON_RETURN:
        if (millis() > last_button_push + BUTTON_REPEAT_RATE) {
          // Serial.println("button2");
          last_button_push = millis();
          Serial1.write(13);
        }
        break;
      }

      // keyboard
      key = GD.inputs.tag;
      if (millis() > last_button_push + BUTTON_REPEAT_RATE
          && (KEY_SPACE <= key) && (key < KEY_DEL)) {
          last_button_push = millis();
        Serial1.write(key);
      }
      prevkey = key;

      // clear the screen
      GD.Clear();
      // draw the text and scrollbar
      terminal.draw();

      // draw some additional buttons
      GD.ColorA(64); // alpha to 64/256
      GD.cmd_bgcolor(VALHALLA);
      GD.cmd_fgcolor(TOPAZ);

      GD.Tag(TAG_BUTTON_BELL);
      GD.cmd_button(10, 12, 40, 30, 18, OPT_FLAT,  "bell");

      GD.Tag(TAG_BUTTON_MACRO);
      GD.cmd_button(60, 12, 50, 30, 18, OPT_FLAT,  "words");

      GD.Tag(TAG_BUTTON_FONT);
      GD.cmd_button(110, 12, 40, 30, 18, OPT_FLAT,  "font");

      GD.ColorA(128); // alpha to 64/256
      GD.cmd_bgcolor(BLACK);
      GD.cmd_fgcolor(DEEP_KOAMARU);

      GD.Tag(TAG_BUTTON_RETURN);
      GD.cmd_button(426, 168 + 25, 16*2-1, 50, 18, OPT_FLAT,  "RET");

      GD.Tag(TAG_BUTTON_BACKSPACE);
      GD.cmd_button(394, 168 - 27, 16*4-1, 24, 18, OPT_FLAT,  "BKSP");

      // draw the keyboard rows
      GD.cmd_keys(200-16, 168 - 52, 16*13, 24, 23, OPT_FLAT | key, "~!@#$%^&*()_+");
      GD.cmd_keys(200-16, 168 - 26, 16*13, 24, 23, OPT_FLAT | key, "`1234567890-=");
      GD.cmd_keys(200,    168,      16*16, 24, 23, OPT_FLAT | key, "qwertyuiop[]\\{}|");
      GD.cmd_keys(200+8,  168 + 26, 16*13, 24, 23, OPT_FLAT | key, "asdfghjkl;':\"");
      GD.cmd_keys(200+16, 168 + 52, 16*13, 24, 23, OPT_FLAT | key, "zxcvbnm,./<>?");

      // draw the spacebar
      GD.Tag(' ');
      GD.cmd_button(308 - 60, 172 + 74, 120, 20, 28, OPT_FLAT,   "");

      // update the screen
      GD.swap();
    }

    // // debug output for serial_buffer overflow
    // if(pending_chars != last_pending_chars) {
    //   Serial.println(pending_chars);
    //   last_pending_chars = pending_chars;
    // }

  } // while(1)
} // loop()

