CC      = arm-none-eabi-gcc
CFLAGS  = -Iinclude -Wall -Wextra -std=c11 -O0 -g -ffreestanding -mcpu=cortex-m3 -mthumb
LDFLAGS = -T linker.ld -nostdlib -mcpu=cortex-m3 -mthumb
TARGET  = build/main.elf
SRCS    = $(wildcard src/*.c)
OBJS    = $(SRCS:src/%.c=build/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -f build/*
