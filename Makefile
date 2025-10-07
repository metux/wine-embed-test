# Makefile for Win32 GUI example built with MinGW (32-bit)

# cross compiler
CC := i686-w64-mingw32-gcc

# output binary
TARGET := win32_subwindows.exe

# source files
SRCS := main.c

# compiler flags
CFLAGS := -Wall -Wextra -O2 -municode

# linker flags (Win32 GUI subsystem)
LDFLAGS := -mwindows

# default target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

# clean target
clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	~/.usr/wine/bin/wine win32_subwindows.exe
