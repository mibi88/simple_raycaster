"""
A quick and dirty raycaster.
mapgen.py: generate C source code from map data.
by Mibi88

This software is licensed under the BSD-3-Clause license:

Copyright (c) 2024 Mibi88.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

from PIL import Image
import sys
import os
import json

INDENT = 4
MAX_COLUMN = 79 # Column 80 for line feed.

column = 4

if len(sys.argv) < 5:
    sys.stderr.write("USAGE: mapgen [MAP] [EXTRADATA] [C SOURCE] [C HEADER]\n")
    sys.exit(1)

infile = sys.argv[1]
extradata = sys.argv[2]
source = sys.argv[3]
header = sys.argv[4]

name = os.path.splitext(os.path.basename(infile))[0]

img = Image.open(infile).convert("RGB")

w, h = img.size

mapdata = []

try:
    data = json.load(open(extradata, "r"))
except:
    sys.stderr.write("mapgen: Invalid extradata JSON!\n")
    sys.exit(0)

tilecolors = []
tiletextures = []

def getcolor(color: str):
    try:
        return int(color[1:], 16)
    except Exception as e:
        print(e)
        sys.stderr.write("mapgen: Invalid color!\n")
        sys.exit(1)

try:
    for i in data["tiles"]:
        tilecolors.append(getcolor(i["color"]))
        tiletextures.append(i["texture"])
except Exception as e:
    print(e)
    sys.stderr.write("mapgen: Invalid extradata!\n")
    sys.exit(1)

try:
    for y in range(h):
        for x in range(w):
            pixel = img.getpixel((x, y))
            color = pixel[0]<<16|pixel[1]<<8|pixel[2]
            idx = 0
            if color != 0xFFFFFF: 
                idx = tilecolors.index(color)+1
            mapdata.append(idx)
except Exception as e:
    print(e)
    sys.stderr.write("mapgen: Invalid extradata!\n")
    sys.exit(1)

out = f"""#include <map.h>
#include <stddef.h>

"""

try:
    headers = []
    for i in data["tiles"]:
        if i["texture"] not in headers:
            headers.append(i["texture"])
    for i in data["sprites"]:
        if i["texture"] not in headers:
            headers.append(i["texture"])
    for i in headers:
        out += f"#include <{i}.h>\n"
except Exception as e:
    print(e)
    sys.stderr.write("mapgen: Invalid extradata!\n")
    sys.exit(1)

out += f"""

Tile {name.lower()}_tileset[{len(data['tiles'])}] = {{
"""

try:
    n = 0
    for i in data["tiles"]:
        out += " "*INDENT
        out += f"{{&{i['texture']}, NULL}},\n"
        n += 1
    out = out[:-2]
except Exception as e:
    print(e)
    sys.stderr.write("mapgen: Invalid extradata!\n")
    sys.exit(1)

out += f"""
}};

Sprite {name.lower()}_sprites[{len(data['sprites'])}] = {{
"""

try:
    sprites = len(data["sprites"])
    for i in data["sprites"]:
        out += " "*INDENT
        out += (f"{{TO_FIXED({i['x']}), TO_FIXED({i['y']}), 0, " +
                f"&{i['texture']}, {int(i['visible'])}, 0, 0, NULL}},\n")
    out = out[:-2]
except Exception as e:
    print(e)
    sys.stderr.write("mapgen: Invalid extradata!\n")
    sys.exit(1)

out += f"""
}};

unsigned char {name.lower()}_data[{w*h}] = {{
"""

out += ' '*INDENT
for n in range(len(mapdata)):
    i = mapdata[n]
    string = f"{hex(i)}, "
    if n >= len(mapdata)-1:
        string = string[:-2]
    if column+len(string) >= MAX_COLUMN:
        out = out[:-1]
        out += '\n'
        out += ' '*INDENT
        column = INDENT
    out += string
    column += len(string)

out += f"""
}};

Map {name.lower()} = {{
    {name.lower()}_data,
    {w}, {h},
    {name.lower()}_tileset,
    {name.lower()}_sprites, {sprites},
    NULL
}};\n
"""

with open(source, "w") as fp:
    fp.write(out)

out = f"""#ifndef {name.upper()}_H
#define {name.upper()}_H

#include <map.h>

extern Map {name.lower()};

#endif\n
"""

with open(header, "w") as fp:
    fp.write(out)

