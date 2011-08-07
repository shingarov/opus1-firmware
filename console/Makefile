#
# Makefile for Ladarevo Opus.1 AVR firmware.
# Copyright (c) 2010-2011 Ladarevo Software Inc.
#

CC=avr-gcc
CXX=avr-g++
INCLUDES=-I/usr/share/arduino/hardware/arduino/cores/arduino

MACHINE_CFLAGS=-mmcu=atmega2560 -DF_CPU=16000000L  # Mega
CFLAGS=$(INCLUDES) -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections $(MACHINE_CFLAGS) -DARDUINO=22
CXXFLAGS=$(CFLAGS)

OBJS=console.o
LIBS=../lib/mega/core.a

COMPORT=/dev/ttyACM0

DUDE=/usr/share/arduino/hardware/tools/avrdude
DUDECONF=/usr/share/arduino/hardware/tools/avrdude.conf
DUDEBUG=-v -v -v -v
MACHINE_DUDEFLAGS=-patmega2560 -cstk500v2 # Mega
DUDEFLAGS=-C$(DUDECONF) $(DUDEBUG) $(MACHINE_DUDEFLAGS) -P$(COMPORT) -b115200 -D 


all: console.hex console.eep

console.elf: $(OBJS)
	avr-gcc -Os -Wl,--gc-sections -mmcu=atmega2560 -o console.elf $(OBJS) $(LIBS) -lm

console.eep: console.elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 console.elf console.eep

console.hex: console.elf
	avr-objcopy -O ihex -R .eeprom console.elf console.hex 

upload: console.hex
	$(DUDE) $(DUDEFLAGS) -Uflash:w:console.hex:i

clean:
	rm -f *.o *.elf *.hex *.eep

