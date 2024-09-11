#!/bin/sh

src="src/render.c src/main.c"

cc $src -o main -Wall -Wextra -Wpedantic -g -Isrc -lSDL2 -lm -ansi

