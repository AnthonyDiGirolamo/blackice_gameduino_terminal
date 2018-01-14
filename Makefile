ARDUINO_DIR = ${HOME}/apps/arduino-1.8.5
SKETCHBOOK	= $(HOME)/Arduino
SKETCH = $(notdir $(CURDIR)).ino
SKETCHDIRNAME = $(notdir $(CURDIR))
TARGET_DIR = $(CURDIR)/build-blackice

all: build upload

serial: build upload monitor

build:
	@ mkdir -p $(TARGET_DIR)

	$(ARDUINO_DIR)/arduino-builder \
	-compile \
	-logger=human \
	-hardware $(ARDUINO_DIR)/hardware \
	-hardware $(HOME)/.arduino15/packages \
	-hardware $(SKETCHBOOK)/hardware \
	-tools $(ARDUINO_DIR)/tools-builder \
	-tools $(ARDUINO_DIR)/hardware/tools/avr \
	-tools $(HOME)/.arduino15/packages \
	-built-in-libraries $(ARDUINO_DIR)/libraries \
	-libraries $(SKETCHBOOK)/libraries \
	-fqbn=millerresearch:stm32l4:BlackIce:usb=cdc,dosfs=sdmmc,opt=o3 \
	-ide-version=10805 \
	-build-path $(TARGET_DIR) \
	-warnings=none \
	-prefs=build.warn_data_percentage=75 \
	-verbose $(SKETCH)

upload:
	$(HOME)/.arduino15/packages/arduino/tools/dfu-util/0.9.0-arduino1/dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -t 1024 -D $(TARGET_DIR)/$(SKETCH).bin

monitor:
	sleep 1 && miniterm.py /dev/ttyACM0 115200

clean:
	rm -rf $(TARGET_DIR)
