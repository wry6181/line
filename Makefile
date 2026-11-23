CC      ?= gcc
CFLAGS  ?= -O2 -Wall
LDLIBS  += -lz -lm
BUILD   := build
TARGET  := $(BUILD)/main
SRCS    := main.c spng.c arena.c string.c
HDRS    := file.h png.h spng.h arena.h string.h types.h
OBJS    := $(SRCS:%.c=$(BUILD)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: %.c $(HDRS) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
