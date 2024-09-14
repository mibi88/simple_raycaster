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
#include <fixed.h>

#include <stdio.h>
#include <stdlib.h>

#define MAP_WIDTH  32
#define MAP_HEIGHT 32

#include <wall.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

#define SCALE 8
#define LEN 25

#define SPEED 5
#define ROTSPEED 100

#define PI 3.14159

#define SHOWPLAYER 1

#define RAYS SCREEN_WIDTH
#define FOV  60

#define TEXTURE 1

#define COLLISIONS 1

typedef struct {
    fixed_t x, y;
} Vector2;

typedef struct {
    fixed_t len;
    int cx, cy;
    unsigned char hit;
    char c;
    char x_axis_hit;
} RayEnd;

Renderer renderer;

char map[MAP_WIDTH*MAP_HEIGHT] = "################################"
                                 "#  #   #                       #"
                                 "#      #                       #"
                                 "###    #           #           #"
                                 "#      #           #           #"
                                 "#  #   #        #  #  #        #"
                                 "#                # # #         #"
                                 "########          ###          #"
                                 "#                  #           #"
                                 "#                              #"
                                 "#                              #"
                                 "#                              #"
                                 "#                              #"
                                 "#                              #"
                                 "#                              #"
                                 "#                #             #"
                                 "#               #              #"
                                 "#              #               #"
                                 "#             #                #"
                                 "#            #                 #"
                                 "#                              #"
                                 "#                              #"
                                 "#                #     #       #"
                                 "#                              #"
                                 "#               #       #      #"
                                 "#                #     #       #"
                                 "#                 #####        #"
                                 "#                              #"
                                 "#                              #"
                                 "#                              #"
                                 "#                              #"
                                 "################################";

/* Player */
fixed_t px = TO_FIXED(1.5);
fixed_t py = TO_FIXED(1.5);
fixed_t pr = TO_FIXED(45);
/* ------ */

char lock = 0;

/* If the fisheye effect should be fixed. */
char fisheye_fix = 0;

char map_view = 1;

Texture *get_tile_tex(int cx, int cy) {
    return &wall;
}

void vline(int y1, int y2, int x, int r, int g, int b) {
    int y;
    for(y=y1;y<y2;y+=y1<y2 ? 1 : -1){
        render_set_pixel(&renderer, x, y, r, g, b, 255);
    }
}

#define ABS(x) ((x) < 0 ? -(x) : (x))

#if TEXTURE
void texline(Texture *tex, int y1, int y2, int ty1, int ty2, int x, int l,
             int fog) {
    int y;
    int r, g, b;
    int p;
    int n;
    int t;
    fixed_t h = ABS(TO_FIXED(ty2)-TO_FIXED(ty1));
    fixed_t texinc = DIV(TO_FIXED(tex->height), (h ? h : 1));
    if(l >= tex->width) l = tex->width-1;
    else if(l < 0) l = 0;
    for(t=y1-ty1,n=0,y=y1;y<y2;y+=y1<y2 ? 1 : -1,n++,t++){
        p = TO_INT(texinc*t);
        if(p < 0) p = 0;
        else if(p >= tex->height) p = tex->height-1;
        r = tex->data[p*tex->width+l]>>24;
        g = (tex->data[p*tex->width+l]>>16)&0xFF;
        b = (tex->data[p*tex->width+l]>>8)&0xFF;
        r = TO_INT(TO_FIXED(r)/255*fog);
        g = TO_INT(TO_FIXED(g)/255*fog);
        b = TO_INT(TO_FIXED(b)/255*fog);
        render_set_pixel(&renderer, x, y, r, g, b, 255);
    }
}
#endif

void line(int x1, int y1, int x2, int y2, int r, int g, int b) {
    int dx = ABS(x2-x1);
    int sx = ((x1 < x2)<<1)-1;
    int dy = -ABS(y2-y1);
    int sy = ((y1 < y2)<<1)-1;
    int error = dx + dy;
    int e2;
    for(;;){
        render_set_pixel(&renderer, x1, y1, r, g, b, 255);
        if(x1 == x2 && y1 == y2){
            break;
        }
        e2 = 2*error;
        if(e2 >= dy){
            if(x1 == x2){
                break;
            }
            error = error+dy;
            x1 = x1 + sx;
        }
        if(e2 <= dx){
            if(y1 == y2){
                break;
            }
            error = error+dx;
            y1 = y1+sy;
        }
    }
}

void rect(int sx, int sy, int w, int h, int r, int g, int b){
    int x, y;
    for(y=0;y<h;y++){
        for(x=0;x<w;x++){
            render_set_pixel(&renderer, sx+x, sy+y, r, g, b, 255);
        }
    }
}

RayEnd raycast(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

void render_map(void) {
    int x, y;
    fixed_t i;
    RayEnd end;
    for(y=0;y<MAP_HEIGHT;y++){
        for(x=0;x<MAP_WIDTH;x++){
            if(map[y*MAP_WIDTH+x] != ' '){
                rect(x*SCALE, y*SCALE, SCALE, SCALE, 0, 0, 0);
            }
        }
    }
    for(i=-(TO_FIXED(FOV)/2);i<TO_FIXED(FOV)/2;i+=TO_FIXED(FOV)/RAYS){
        end = raycast(px, py, px+dcos(pr+i)*LEN, py+dsin(pr+i)*LEN);
        /*printf("%f\n", end.len/(float)(1<<PRECISION));*/
        if(fisheye_fix) end.len = MUL(end.len, dcos(i));
        line(TO_INT(px*SCALE), TO_INT(py*SCALE),
             TO_INT((px+MUL(dcos(pr+i), end.len))*SCALE),
             TO_INT((py+MUL(dsin(pr+i), end.len))*SCALE), 0, 255, 0);
    }
#if SHOWPLAYER
    line(TO_INT(px*SCALE), TO_INT(py*SCALE), TO_INT((px+dcos(pr))*SCALE),
         TO_INT((py+dsin(pr))*SCALE), 255, 0, 0);
#endif
}

void render_world(void) {
    fixed_t i;
    int p;
    int c;
    int h;
    fixed_t l;
    int no_clip_h;
    RayEnd end;
    Texture *tex;
    for(i=-(TO_FIXED(FOV)/2),p=0;i<TO_FIXED(FOV)/2;i+=TO_FIXED(FOV)/RAYS,
        p+=SCREEN_WIDTH/RAYS){
        end = raycast(px, py, px+dcos(pr+i)*LEN, py+dsin(pr+i)*LEN);
        if(!end.hit){
            for(c=0;c<SCREEN_WIDTH/RAYS;c++){
                vline(0, SCREEN_HEIGHT, p+c, 0, 0, 0);
            }
            continue;
        }
        tex = get_tile_tex(end.cx, end.cy);
        if(end.x_axis_hit){
            l = px+MUL(dcos(pr+i), end.len);
            l = l-FLOOR(l);
            l *= tex->width;
        }else{
            l = py+MUL(dsin(pr+i), end.len);
            l = l-FLOOR(l);
            l *= tex->width;
        }
        if(fisheye_fix) end.len = MUL(end.len, dcos(i));
        h = TO_INT(DIV(TO_FIXED(SCREEN_HEIGHT), (end.len ? end.len : 1)));
        no_clip_h = h;
        if(h > SCREEN_HEIGHT) h = SCREEN_HEIGHT;
        for(c=0;c<SCREEN_WIDTH/RAYS;c++){
            vline(0, SCREEN_HEIGHT/2-h/2, p+c, 0, 0, 0);
            vline(SCREEN_HEIGHT/2+h/2, SCREEN_HEIGHT, p+c, 0, 0, 0);
#if TEXTURE
            texline(tex, SCREEN_HEIGHT/2-h/2, SCREEN_HEIGHT/2+h/2, 
                    SCREEN_HEIGHT/2-no_clip_h/2, SCREEN_HEIGHT/2+no_clip_h/2,
                    p+c, TO_INT(l), 255-TO_INT(end.len/LEN*255));
#else
            vline(SCREEN_HEIGHT/2-h/2, SCREEN_HEIGHT/2+h/2, p+c,
                  (255-TO_INT(end.len/LEN*255))*(!end.x_axis_hit),
                  (255-TO_INT(end.len/LEN*255))*end.x_axis_hit, 0);
#endif
        }
    }
}

RayEnd raycast(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2) {
    int px = TO_INT(x1);
    int py = TO_INT(y1);
    fixed_t tmp;
    RayEnd end;
    Vector2 rays;
    Vector2 raystep;
    Vector2 steplen;
    Vector2 start;
    tmp = ABS(x2-x1);
    if(tmp) raystep.x = DIV((y2-y1), tmp);
    else raystep.x = (y2-y1);
    steplen.x = fsqrt(MUL(raystep.x, raystep.x)+TO_FIXED(1));
    tmp = ABS(y2-y1);
    if(tmp) raystep.y = DIV((x2-x1), tmp);
    else raystep.y = (x2-x1);
    steplen.y = fsqrt(MUL(raystep.y, raystep.y)+TO_FIXED(1));
    end.hit = 0;
    /* Calculate start */
    if(x1 < x2){
        start.x = TO_FIXED(1)-(x1-FLOOR(x1));
    }else{
        start.x = x1-FLOOR(x1);
    }
    if(y1 < y2){
        start.y = TO_FIXED(1)-(y1-FLOOR(y1));
    }else{
        start.y = y1-FLOOR(y1);
    }
    /* --------------- */
    rays.y = MUL(steplen.y, start.y);
    rays.x = MUL(steplen.x, start.x);
    if(rays.x < rays.y){
        end.len = rays.x;
        end.x_axis_hit = 1;
    }else{
        end.len = rays.y;
        end.x_axis_hit = 0;
    }
    while(end.len < TO_FIXED(LEN)){
        /*rect(px*SCALE, py*SCALE, SCALE, SCALE, 127, 127, 255);*/
        if(rays.x < rays.y){
            px += x1 < x2 ? 1 : -1;
            rays.x += steplen.x;
            end.x_axis_hit = 0;
        }else{
            py += y1 < y2 ? 1 : -1;
            rays.y += steplen.y;
            end.x_axis_hit = 1;
        }
        if(px >= 0 && px < MAP_WIDTH && py >= 0 && py < MAP_HEIGHT){
            end.cx = px;
            end.cy = py;
            if(map[py*MAP_WIDTH+px] != ' '){
                end.hit = 1;
                break;
            }
        }else{
            end.cx = px;
            end.cy = py;
            end.hit = 0;
            break;
        }
        if(rays.x < rays.y){
            end.len = rays.x;
        }else{
            end.len = rays.y;
        }
    }
    return end;
}

void loop(int fps) {
    RayEnd end;
    fixed_t oldx;
    fixed_t oldy;
    int tx, ty;
    printf("FPS: %d    \r", fps);
    fflush(stdout);
    if(render_keydown(&renderer, KEY_LEFT)) pr -= TO_FIXED(ROTSPEED)/fps;
    if(render_keydown(&renderer, KEY_RIGHT)) pr += TO_FIXED(ROTSPEED)/fps;
    if(render_keydown(&renderer, KEY_UP)){
        oldx = px;
        oldy = py;
        px += MUL(dcos(pr), DIV(TO_FIXED(SPEED), TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(px);
        ty = TO_INT(py);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                px = oldx;
            }
        }else{
            px = oldx;
        }
#endif
        py += MUL(dsin(pr), DIV(TO_FIXED(SPEED), TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(px);
        ty = TO_INT(py);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                py = oldy;
            }
        }else{
            py = oldy;
        }
#endif
    }
    if(render_keydown(&renderer, KEY_DOWN)){
        oldx = px;
        oldy = py;
        px -= MUL(dcos(pr), DIV(TO_FIXED(SPEED), TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(px);
        ty = TO_INT(py);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                px = oldx;
            }
        }else{
            px = oldx;
        }
#endif
        py -= MUL(dsin(pr), DIV(TO_FIXED(SPEED), TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(px);
        ty = TO_INT(py);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                py = oldy;
            }
        }else{
            py = oldy;
        }
#endif
    }
    if(!lock){
        if(render_keydown(&renderer, KEY_LCTRL)){
            map_view = !map_view;
            lock = 1;
        }
        if(render_keydown(&renderer, KEY_SPACE)){
            fisheye_fix = !fisheye_fix;
            lock = 1;
        }
    }else{
        lock = render_keydown(&renderer, KEY_LCTRL) |
               render_keydown(&renderer, KEY_SPACE);
    }
    render_clear(&renderer);
    if(map_view){
        render_map();
    }else{
        render_world();
    }
    render_update(&renderer);
}

int main(int argc, char **argv) {
    render_init(&renderer, SCREEN_WIDTH, SCREEN_HEIGHT, "DDA Test");
    render_main_loop(&renderer, loop);
    return 0;
}

