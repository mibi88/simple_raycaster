#!/bin/sh

src="platforms/sdl2/render.c src/main.c src/fixed.c src/raycaster.c src/map.c \
     conv/wall.c conv/wood.c conv/sprite.c conv/testmap.c"

mkdir -p conv

python3 src/texgen.py assets/wall.png conv/wall.c conv/wall.h
python3 src/texgen.py assets/wood.png conv/wood.c conv/wood.h
python3 src/texgen.py assets/sprite.png conv/sprite.c conv/sprite.h

python3 src/mapgen.py assets/testmap.png assets/testmap.json conv/testmap.c \
        conv/testmap.h

cc $src -o main -Wall -Wextra -Wpedantic -g -Isrc -Iplatforms/sdl2 -Iconv \
        -lSDL2 -lm -ansi

