EMU_BINARY=pcpu-emu
EMU_SOURCES= emu-dump.c emu.c
EMU_CFLAGS=-std=c99 -lSDL

emu:
	$(CC) -o $(EMU_BINARY) $(EMU_SOURCES) $(EMU_CFLAGS)

all: emu
