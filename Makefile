
TARGET=main
CC=gcc
CCFLAGS=-std=c11
BUILD_DIR=build
HEADERS=private.h

all: $(TARGET)
clean:
	rm -f $(TARGET)
	rm -f $(BUILD_DIR)/main.o

$(BUILD_DIR):
	mkdir -p "$(BUILD_DIR)"

$(BUILD_DIR)/main.o: main.c $(BUILD_DIR) $(HEADERS)
	$(CC) $(CCFLAGS) -c main.c \
		-o $(BUILD_DIR)/main.o

$(TARGET): $(BUILD_DIR)/main.o
	$(CC) $(BUILD_DIR)/main.o \
		-framework CoreGraphics \
		-framework OpenGL \
		-o $(TARGET)
