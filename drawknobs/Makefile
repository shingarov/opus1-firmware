#
# Makefile for Ladarevo Opus.1 AVR firmware.
# Copyright (c) 2010-2011 Ladarevo Software Inc.
#

CC=avr-gcc
CXX=avr-g++
INCLUDES=-I/usr/share/arduino/hardware/arduino/cores/arduino
CFLAGS=$(INCLUDES) -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=22
CXXFLAGS=$(CFLAGS)

OBJS=drawknobs.o

DUDE=/usr/share/arduino/hardware/tools/avrdude
DUDECONF=/usr/share/arduino/hardware/tools/avrdude.conf
DUDEBUG=-v -v -v -v
DUDEFLAGS=-C$(DUDECONF) $(DUDEBUG) -patmega328p -carduino -P/dev/ttyUSB0 -b57600 -D 


all: drawknobs.hex drawknobs.eep

drawknobs.elf: $(OBJS) core.a
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega328p -o drawknobs.elf $(OBJS) core.a -lm

drawknobs.eep: drawknobs.elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 drawknobs.elf drawknobs.eep

drawknobs.hex: drawknobs.elf
	avr-objcopy -O ihex -R .eeprom drawknobs.elf drawknobs.hex 

upload: drawknobs.hex
	$(DUDE) $(DUDEFLAGS) -Uflash:w:drawknobs.hex:i



clean:
	rm -f *.o *.elf *.hex *.eep

