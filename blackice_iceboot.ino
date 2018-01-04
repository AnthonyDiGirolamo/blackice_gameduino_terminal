/*
 * Configure ICE40 with bitstream from offset 0x0801F000 in STM32 flash
 * (if present), and then repeatedly from Serial (USB CDC-ACM) port
 */

#include <MyStorm.h>

void setup()
{
  Serial.begin(115200);
  while (!Serial) ; // wait for USB serial connection to be established
  Serial.println("setup() start");
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  myStorm.FPGAConfigure((const byte*)0x0801F000, 135100);
  digitalWrite(LED_BUILTIN, 0);
  Serial.println("setup() done");
}

void loop()
{
  if (Serial.available()) {
    digitalWrite(LED_BUILTIN, 1);
    if (myStorm.FPGAConfigure(Serial)) {
      while (Serial.available())
        Serial.read();
      Serial.println("FPGAConfigure done");
    }
    digitalWrite(LED_BUILTIN, 0);
  }
  else if (Serial1.available() > 0) {
    Serial.write(Serial1.read());
  }
  // else
  //   return;
}

