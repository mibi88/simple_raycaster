#!/bin/sh

src="platforms/sdl2/render.c src/main.c src/fixed.c src/raycaster.c \
     conv/wall.c conv/sprite.c"

mkdir -p conv

python3 src/texgen.py assets/wall.png conv/wall.c conv/wall.h
python3 src/texgen.py assets/sprite.png conv/sprite.c conv/sprite.h

cc $src -o main -Wall -Wextra -Wpedantic -g -Isrc -Iplatforms/sdl2 -Iconv \
        -lSDL2 -lm -ansi

