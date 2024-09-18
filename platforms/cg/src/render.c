/* A quick and dirty raycaster.
 * by Mibi88
 *
 * This software is licensed under the BSD-3-Clause license:
 *
 * Copyright 2024 Mibi88
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <render.h>
#include <gint/keyboard.h>
#include <gint/display.h>
#include <gint/timer.h>

#define COLOR_FIX1 (DIV(TO_FIXED(15), TO_FIXED(255)))
#define COLOR_FIX2 (DIV(TO_FIXED(31), TO_FIXED(255)))

void render_init(Renderer *renderer, int width, int height, char *title) {
    dclear(C_WHITE);
}

void render_set_pixel(Renderer *renderer, int x, int y, int r, int g, int b) {
    r = TO_INT(r*COLOR_FIX1);
    g = TO_INT(g*COLOR_FIX2);
    b = TO_INT(b*COLOR_FIX1);
    dpixel(x, y, C_RGB(r, g, b));
}

void render_line(Renderer *renderer, int x1, int y1, int x2, int y2, int r,
                 int g, int b) {
     r = TO_INT(r*COLOR_FIX1);
     g = TO_INT(g*COLOR_FIX2);
     b = TO_INT(b*COLOR_FIX1);
    dline(x1, y1, x2, y2, C_RGB(r, g, b));
}

void render_rect(Renderer *renderer, int sx, int sy, int w, int h, int r,
                 int g, int b) {
     r = TO_INT(r*COLOR_FIX1);
     g = TO_INT(g*COLOR_FIX2);
     b = TO_INT(b*COLOR_FIX1);
    drect(sx, sy, sx+w, sy+h, C_RGB(r, g, b));
}

void render_vline(Renderer *renderer, int y1, int y2, int x, int r, int g,
                  int b) {
    int y;
    unsigned short int c;
    r = TO_INT(r*COLOR_FIX1);
    g = TO_INT(g*COLOR_FIX2);
    b = TO_INT(b*COLOR_FIX1);
    c = C_RGB(r, g, b);
    if(x < 0 || x >= DWIDTH) return;
    if(y1 < 0) y1 = 0;
    else if(y1 >= DHEIGHT) y1 = DHEIGHT-1;
    if(y2 >= DHEIGHT) y2 = DHEIGHT-1;
    else if(y2 < 0) y2 = 0;
    for(y=y1;y<y2;y+=y1<y2 ? 1 : -1){
        gint_vram[y*DWIDTH+x] = c;
    }
}

void render_texvline(Renderer *renderer, Texture *tex, int y1, int y2, int ty1,
                     int ty2, int x, int l, int fog) {
    int y;
    uint16_t c;
    int p;
    int n;
    int t;
    uint16_t tmp;
    fixed_t m = TO_FIXED(fog)/255;
    unsigned int h = ABS(ty2-ty1);
    ufixed_t texinc = UTO_FIXED(tex->height)/(h ? h : 1);
    if(x < 0 || x >= DWIDTH) return;
    if(y1 < 0) y1 = 0;
    else if(y1 >= DHEIGHT) y1 = DHEIGHT-1;
    if(y2 >= DHEIGHT) y2 = DHEIGHT-1;
    else if(y2 < 0) y2 = 0;
    if(l >= tex->width) l = tex->width-1;
    else if(l < 0) l = 0;
    for(t=y1-ty1,n=0,y=y1;y<y2;y+=y1<y2 ? 1 : -1,n++,t++){
        p = UTO_INT(texinc*t);
        if(p < 0) p = 0;
        else if(p >= tex->height) p = tex->height-1;
        c = ((uint16_t*)tex->data)[p*tex->width+l];
        tmp = c;
        c = TO_INT((tmp&0x1F)*m)&0x1F;
        c |= (TO_INT(((tmp>>5)&0x3F)*m)&0x3F)<<5;
        c |= (TO_INT(((tmp>>11)&0x1F)*m)&0x1F)<<11;
        gint_vram[y*DWIDTH+x] = c;
    }
}

void render_update(Renderer *renderer) {
    dupdate();
}

void render_clear(Renderer *renderer, char black) {
    dclear(black ? C_BLACK : C_WHITE);
}

char render_keydown(Renderer *renderer, int key) {
    clearevents();
    return keydown(key);
}

int render_get_width(Renderer *renderer) {
    return DWIDTH;
}

int render_get_height(Renderer *renderer) {
    return DHEIGHT;
}

int render_ms(Renderer *renderer) {
    return 0; /* TODO */
}

int _fps;

void render_show_fps(Renderer *renderer) {
    dprint(8, 8, C_LIGHT, "FPS: %d", _fps);
}

int _ms_time;

int timer_call(void) {
    _ms_time++;
    return TIMER_CONTINUE;
}

void render_main_loop(Renderer *renderer, void (*loop_function)(int)) {
    int timer = timer_configure(TIMER_TMU, 1000, GINT_CALL(timer_call));
    timer_start(timer);
    clearevents();
    _fps = 30;
    _ms_time = 0;
    while(!keydown(KEY_EXIT)){
        loop_function(_fps ? _fps : 1);
        _fps = 1000/(_ms_time ? _ms_time : 1);
        _ms_time = 0;
        clearevents();
    }
    timer_stop(timer);
}
