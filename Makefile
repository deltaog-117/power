CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -D_GNU_SOURCE
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/power

SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

INCLUDES = -I$(SRC_DIR)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/power

uninstall:
	rm -f /usr/local/bin/power

test:
	@echo "No tests implemented yet"

.PHONY: all clean install uninstall test
