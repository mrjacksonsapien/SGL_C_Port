CC = gcc
CFLAGS = -Wall -I x86_64-w64-mingw32/include
LDFLAGS = -L x86_64-w64-mingw32/lib -lSDL3

SRC_DIR = x86_64-w64-mingw32/src
BIN_DIR = x86_64-w64-mingw32/bin
TARGET = $(BIN_DIR)/main.exe

SRCS = $(wildcard $(SRC_DIR)/*.c)

all:
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LDFLAGS)
	@$(TARGET)

clean:
	@if exist "$(TARGET)" del /q "$(TARGET)"