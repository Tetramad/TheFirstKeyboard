SHELL := bash

MCU := atmega32u4
PARTNO := m32u4
PROGRAMMER := avrispmkII

CC = avr-gcc
CFLAGS += -std=c2x -Wall -Wextra -Wpedantic -Werror
CFLAGS += -Wno-array-bounds -Wno-gnu-binary-literal
CFLAGS += -O2
CFLAGS += -DF_CPU=16000000UL -DUSART_BAUDRATE=19200
CFLAGS += -mmcu=$(MCU)

.PHONY: all program build compile clean

all: program

program: a.out
	avrdude -p $(PARTNO) -c $(PROGRAMMER) -U flash:w:$<:e

build: a.out

compile: main.o

clean:
	rm -f -- *.out *.bin *.o

a.out: main.o usb.o
	$(CC) $(CFLAGS) $^
