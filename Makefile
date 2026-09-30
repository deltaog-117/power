CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -D_GNU_SOURCE
LDFLAGS =

# MENU selects the built-in menu backends compiled in:
#   launcher (default)  no extra dependencies, external launchers only
#   builtin             X11 menu (needs libX11, libXft, libXinerama headers)
#   wayland             Wayland menu (needs wayland-client, xkbcommon, cairo,
#                       pango and wayland-scanner; uses wlr-layer-shell)
#   both                X11 and Wayland, chosen at runtime by the session
MENU ?= launcher
X11_LIBS = x11 xft xinerama
WAYLAND_LIBS = wayland-client xkbcommon cairo pangocairo

SRC_DIR = src
BUILD_ROOT = build
BUILD_DIR = $(BUILD_ROOT)/$(MENU)
BIN_DIR = bin
TARGET = $(BIN_DIR)/power
BUILT_BIN = $(BUILD_DIR)/power.bin

WITH_X11 = $(filter $(MENU),builtin both)
WITH_WAYLAND = $(filter $(MENU),wayland both)

SOURCES = $(shell find $(SRC_DIR) -name '*.c')
PROTO_OBJECTS =
ifneq ($(WITH_X11),)
CFLAGS += -DPOWER_BUILTIN_MENU -DPOWER_MENU_X11 $(shell pkg-config --cflags $(X11_LIBS))
LDFLAGS += $(shell pkg-config --libs $(X11_LIBS))
else
SOURCES := $(filter-out %/menu_x11.c,$(SOURCES))
endif
ifneq ($(WITH_WAYLAND),)
PROTO_DIR = $(BUILD_DIR)/protocols
WAYLAND_PROTOCOLS_DIR = $(shell pkg-config --variable=pkgdatadir wayland-protocols)
XDG_SHELL_XML = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml
LAYER_SHELL_XML = protocols/wlr-layer-shell-unstable-v1.xml
PROTO_HEADERS = $(PROTO_DIR)/xdg-shell-client-protocol.h \
                $(PROTO_DIR)/wlr-layer-shell-unstable-v1-client-protocol.h
PROTO_OBJECTS = $(PROTO_DIR)/xdg-shell-protocol.o \
                $(PROTO_DIR)/wlr-layer-shell-unstable-v1-protocol.o
CFLAGS += -DPOWER_BUILTIN_MENU -DPOWER_MENU_WAYLAND -I$(PROTO_DIR) $(shell pkg-config --cflags $(WAYLAND_LIBS))
LDFLAGS += $(shell pkg-config --libs $(WAYLAND_LIBS))
else
SOURCES := $(filter-out %/menu_wayland.c,$(SOURCES))
endif
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) $(PROTO_OBJECTS)

INCLUDES = -I$(SRC_DIR)

# Track header dependencies so a changed struct (e.g. PowerConfig) rebuilds
# every object that includes it instead of linking stale layouts together.
CFLAGS += -MMD -MP

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

ifneq ($(WITH_WAYLAND),)
# The layer-shell XML is vendored in protocols/; xdg-shell comes from the
# wayland-protocols package because layer-shell's popup request refers to it.
$(PROTO_DIR)/xdg-shell-client-protocol.h: $(XDG_SHELL_XML)
	@mkdir -p $(PROTO_DIR)
	wayland-scanner client-header $< $@

$(PROTO_DIR)/xdg-shell-protocol.c: $(XDG_SHELL_XML)
	@mkdir -p $(PROTO_DIR)
	wayland-scanner private-code $< $@

$(PROTO_DIR)/wlr-layer-shell-unstable-v1-client-protocol.h: $(LAYER_SHELL_XML)
	@mkdir -p $(PROTO_DIR)
	wayland-scanner client-header $< $@

$(PROTO_DIR)/wlr-layer-shell-unstable-v1-protocol.c: $(LAYER_SHELL_XML)
	@mkdir -p $(PROTO_DIR)
	wayland-scanner private-code $< $@

$(PROTO_DIR)/%.o: $(PROTO_DIR)/%.c $(PROTO_HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/power/repository/menu_wayland.o: $(PROTO_HEADERS)
endif

clean:
	rm -rf $(BUILD_ROOT) $(BIN_DIR)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/power

uninstall:
	rm -f /usr/local/bin/power

test:
	@echo "No tests implemented yet"

-include $(OBJECTS:.o=.d)

.PHONY: all clean install uninstall test $(TARGET)
