#include <FS.h>
#include <MyStorm.h>
#include <SPI.h>
#include <GD2.h>
#include "circular_buffer.h"
#include "terminal.h"

uint32_t profile_time = 0;

Terminal terminal;

circular_buffer<uint8_t> serial_buffer(512);

void get_data() {
  __disable_irq();
  while (Serial1.available() > 0) {
    serial_buffer.put(Serial1.read());
  }
  __enable_irq();
}

void setup() {
  GD.begin();

  Serial.begin(115200);
  Serial1.begin(115200);

  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, 1);
  // myStorm.FPGAConfigure((const byte*)0x0801F000, 135100);
  // digitalWrite(LED_BUILTIN, 0);

  terminal.append_string("Welcome!\n\n");

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

  Serial1.onReceive(get_data);
}

uint8_t result;
uint32_t last_button_push;

void loop() {
  while (1) {
    profile_time = micros();

  // // Random Characters test
  // char newchar;
  // if (terminal.line_count < 10) { //terminal.scrollback_length) {
  //   for(int i=0; i<40; i++) {
  //     newchar = (char) GD.random(256);
  //     terminal.append_character(newchar);
  //     // sprintf(linebuffer, "%-3u", terminal.line_count);
  //   }
  // }


  // ISR get_data approach
  while (!serial_buffer.empty()) {
    result = terminal.append_character(serial_buffer.get());
    if (result == LINE_FULL)
      break;
  }

  if (serial_buffer.empty()) {
    // partial line needs sending
    terminal.upload_to_graphics_ram();
  }


  // while (Serial1.available() > 0) {
  //   newchar1 = Serial1.read();
  //   terminal.append_character(newchar1);
  //   // Serial.write(newchar1);
  // }
  // terminal.upload_to_graphics_ram();

  // while(Serial.available() > 0) {
  //   newchar2 = Serial.read();
  //   // terminal.append_character(newchar2);
  //   Serial1.write(newchar2);
  // }

  GD.get_inputs();
  switch (GD.inputs.track_tag & 0xff) {
  case TAG_SCROLLBAR:
    terminal.update_scrollbar_position(GD.inputs.track_val);
    break;
  case TAG_BUTTON1:
    if (millis() > last_button_push + 200) {
      Serial1.write("words");
      Serial1.write(13);
      last_button_push = millis();
    }
    break;
  case TAG_BUTTON2:
    if (millis() > last_button_push + 200) {
      Serial1.write(13);
      last_button_push = millis();
    }
    break;
  }

  GD.Clear();
  terminal.draw();
  GD.Tag(TAG_BUTTON1);
  GD.cmd_button(352, 12, 40, 30, 18, OPT_FLAT,  "words");
  GD.Tag(TAG_BUTTON2);
  GD.cmd_button(400, 12, 40, 30, 18, OPT_FLAT,  "Enter");
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

  } // while(1)

}

