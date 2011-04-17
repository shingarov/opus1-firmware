#
# Makefile for Ladarevo Opus.1 AVR firmware.
# Copyright (c) 2010-2011 Ladarevo Software Inc.
#

CXX=avr-g++
INCLUDES=-I/usr/share/arduino/hardware/arduino/cores/arduino
CXXFLAGS=$(INCLUDES) -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=22

OBJS=opus1.o PS2Keyboard.o

DUDE=/usr/share/arduino/hardware/tools/avrdude
DUDECONF=/usr/share/arduino/hardware/tools/avrdude.conf
DUDEBUG=-v -v -v -v
DUDEFLAGS=-C$(DUDECONF) $(DUDEBUG) -patmega2560 -cstk500v2 -P/dev/ttyACM0 -b115200 -D 


all: opus1.hex opus1.eep

opus1.elf: $(OBJS) core.a
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega2560 -o opus1.elf $(OBJS) core.a -lm

opus1.eep: opus1.elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 opus1.elf opus1.eep

opus1.hex: opus1.elf
	avr-objcopy -O ihex -R .eeprom opus1.elf opus1.hex 

upload: opus1.hex
	$(DUDE) $(DUDEFLAGS) -Uflash:w:opus1.hex:i



clean:
	rm -f *.o *.elf *.hex *.eep

