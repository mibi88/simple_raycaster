#!/bin/sh

src="src/render.c src/main.c conv/wall.c"

mkdir -p conv

python3 src/texgen.py assets/wall.png conv/wall.c conv/wall.h

cc $src -o main -Wall -Wextra -Wpedantic -g -Isrc -Iconv -lSDL2 -lm -ansi

