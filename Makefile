CC = gcc
CFLAGS = -Wall -Wextra -std=c11 `pkg-config --cflags sdl2 SDL2_image SDL2_ttf`
LIBS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf`
SRC = src/main.c
OUT = save-duffy

all:
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LIBS)

clean:
	rm -f $(OUT)
.PHONY: all clean
