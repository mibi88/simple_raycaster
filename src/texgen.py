"""
A quick and dirty raycaster.
texgen.py: generate an array of pixel data.
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

if len(sys.argv) < 2:
    sys.stderr.write("USAGE: texgen [FILE]\n")
    sys.exit(1)

infile = sys.argv[1]

name = os.path.splitext(os.path.basename(infile))[0]

img = Image.open(infile).convert("RGBA")

w, h = img.size

pxlist = []

for y in range(h):
    for x in range(w):
        pixel = img.getpixel((x, y))
        pxlist.append(pixel[0]<<24|pixel[1]<<16|pixel[2]<<8|pixel[3])

out = f"""#define {name.upper()}_WIDTH {w}
#define {name.upper()}_HEIGHT {h}

unsigned int {name.lower()}[{name.upper()}_WIDTH*{name.upper()}_HEIGHT] = {{
"""

INDENT = 4
MAX_COLUMN = 79 # Column 80 for line feed.

column = 4

out += ' '*INDENT

for n in range(len(pxlist)):
    i = pxlist[n]
    string = f"{hex(i)}, "
    if n >= len(pxlist)-1:
        string = string[:-2]
    if column+len(string) >= MAX_COLUMN:
        out = out[:-1]
        out += '\n'
        out += ' '*INDENT
        column = INDENT
    out += string
    column += len(string)

out += """
};
"""

sys.stdout.write(out)

