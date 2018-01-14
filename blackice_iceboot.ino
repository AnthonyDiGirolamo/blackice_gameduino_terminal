#include <FS.h>
#include <MyStorm.h>
#include <SPI.h>
#include <GD2.h>
#include "circular_buffer.h"
#include "terminal.h"

#define TAG_BUTTON1 202
#define TAG_BUTTON2 203

#define BUTTON_REPEAT_RATE 200
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
  terminal.append_string("Welcome!\n\n");

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
      case TAG_BUTTON1:
        if (millis() > last_button_push + BUTTON_REPEAT_RATE) {
          // Serial.println("button1");
          last_button_push = millis();
          Serial1.write("words");
        }
        break;
      case TAG_BUTTON2:
        if (millis() > last_button_push + BUTTON_REPEAT_RATE) {
          // Serial.println("button2");
          last_button_push = millis();
          Serial1.write(13);
        }
        break;
      }

      // keyboard
      key = GD.inputs.tag;
      // if ' ' <= key < 0x7f (delete)
      if ((prevkey == 0x00) && (' ' <= key) && (key < 0x7f)) {
        Serial1.write(key);
      }
      prevkey = key;

      // clear the screen
      GD.Clear();
      // draw the text and scrollbar
      terminal.draw();

      // draw some additional buttons
      GD.Tag(TAG_BUTTON1);
      GD.cmd_button(10, 12, 50, 30, 18, OPT_FLAT,  "words");

      GD.Tag(TAG_BUTTON2);
      GD.cmd_button(426, 168 + 25, 31, 50, 18, OPT_FLAT,  "RET");

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

