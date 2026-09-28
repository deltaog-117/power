CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -D_GNU_SOURCE
LDFLAGS =

# MENU=builtin adds the built-in X11 menu (needs libX11, libXft and
# libXinerama headers); the default build has no extra dependencies.
MENU ?= launcher
MENU_LIBS = x11 xft xinerama

SRC_DIR = src
BUILD_ROOT = build
BUILD_DIR = $(BUILD_ROOT)/$(MENU)
BIN_DIR = bin
TARGET = $(BIN_DIR)/power
BUILT_BIN = $(BUILD_DIR)/power.bin

SOURCES = $(shell find $(SRC_DIR) -name '*.c')
ifeq ($(MENU),builtin)
CFLAGS += -DPOWER_BUILTIN_MENU $(shell pkg-config --cflags $(MENU_LIBS))
LDFLAGS += $(shell pkg-config --libs $(MENU_LIBS))
else
SOURCES := $(filter-out %/menu_x11.c,$(SOURCES))
endif
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

INCLUDES = -I$(SRC_DIR)

all: $(TARGET)

$(BUILT_BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Each MENU variant builds in its own directory, so switching variants never
# reuses objects compiled with different flags. The copy runs every time
# (phony) because a variant's binary can be older than the one in bin/.
$(TARGET): $(BUILT_BIN)
	@mkdir -p $(BIN_DIR)
	@cmp -s $< $@ || cp $< $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_ROOT) $(BIN_DIR)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/power

uninstall:
	rm -f /usr/local/bin/power

test:
	@echo "No tests implemented yet"

.PHONY: all clean install uninstall test $(TARGET)
