#
# Makefile for Ladarevo Opus.1 AVR firmware.
# Copyright (c) 2010-2011 Ladarevo Software Inc.
#

CC=avr-gcc
CXX=avr-g++
INCLUDES=-I/usr/share/arduino/hardware/arduino/cores/arduino -I/usr/share/arduino/libraries/Sprite -I/usr/share/arduino/libraries/Matrix
CFLAGS=$(INCLUDES) -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -mmcu=atmega2560 -DF_CPU=16000000L -DARDUINO=22
CXXFLAGS=$(CFLAGS)

OBJS=opus1.o LED.o
LIBS=lib/Matrix.o lib/Sprite.o lib/core.a

COMPORT=/dev/ttyACM0

DUDE=/usr/share/arduino/hardware/tools/avrdude
DUDECONF=/usr/share/arduino/hardware/tools/avrdude.conf
DUDEBUG=-v -v -v -v
DUDEFLAGS=-C$(DUDECONF) $(DUDEBUG) -patmega2560 -cstk500v2 -P$(COMPORT) -b115200 -D 


all: opus1.hex opus1.eep

opus1.elf: $(OBJS)
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega2560 -o opus1.elf $(OBJS) $(LIBS) -lm

opus1.eep: opus1.elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 opus1.elf opus1.eep

opus1.hex: opus1.elf
	avr-objcopy -O ihex -R .eeprom opus1.elf opus1.hex 

upload: opus1.hex
	$(DUDE) $(DUDEFLAGS) -Uflash:w:opus1.hex:i

clean:
	rm -f *.o *.elf *.hex *.eep

