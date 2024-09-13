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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAP_WIDTH  8
#define MAP_HEIGHT 8

#define TEX_WIDTH  8
#define TEX_HEIGHT 8

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

#define SCALE 64
#define LEN 15

#define SPEED 5
#define ROTSPEED 100

#define PI 3.14159

#define SHOWPLAYER 1

#define RAYS SCREEN_WIDTH
#define FOV  60

#define TEXTURE 1

typedef struct {
    float x, y;
} Vector2;

typedef struct {
    float len;
    int cx, cy;
    unsigned char hit;
    char c;
    char x_axis_hit;
} RayEnd;

Renderer renderer;

char map[MAP_WIDTH*MAP_HEIGHT] = "########"
                                 "#  #   #"
                                 "#      #"
                                 "###    #"
                                 "#      #"
                                 "#  #   #"
                                 "#      #"
                                 "########";

char tex[TEX_WIDTH*TEX_HEIGHT] = "########"
                                 "#      #"
                                 "#      #"
                                 "#      #"
                                 "#      #"
                                 "#      #"
                                 "#      #"
                                 "########";

/* Player */
float px = 1.5;
float py = 1.5;
float pr = 45;
/* ------ */

char lock = 0;

/* If the fisheye effect should be fixed. */
char fisheye_fix = 0;

char map_view = 1;

void vline(int y1, int y2, int x, int r, int g, int b) {
    int y;
    for(y=y1;y<y2;y+=y1<y2 ? 1 : -1){
        render_set_pixel(&renderer, x, y, r, g, b, 255);
    }
}

#if TEXTURE
void texline(int y1, int y2, int x, int l) {
    int y;
    int c;
    int p;
    int n;
    float texinc = TEX_HEIGHT/fabs(y2-y1);
    if(l >= TEX_WIDTH) l = TEX_WIDTH-1;
    else if(l < 0) l = 0;
    for(n=0,y=y1;y<y2;y+=y1<y2 ? 1 : -1,n++){
        p = texinc*n;
        if(p < 0) p = 0;
        else if(p >= TEX_HEIGHT) p = TEX_HEIGHT-1;
        c = 128+(tex[p*TEX_WIDTH+l] == ' ')*127;
        render_set_pixel(&renderer, x, y, c, c, c, 255);
    }
}
#endif

#define ABS(x) ((x) < 0 ? -(x) : (x))

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

RayEnd raycast(float x1, float y1, float x2, float y2);

void render_map(void) {
    int x, y;
    float i;
    RayEnd end;
    for(y=0;y<MAP_HEIGHT;y++){
        for(x=0;x<MAP_WIDTH;x++){
            if(map[y*MAP_WIDTH+x] != ' '){
                rect(x*SCALE, y*SCALE, SCALE, SCALE, 0, 0, 0);
            }
        }
    }
    for(i=-(FOV/2);i<FOV/2;i+=FOV/(float)RAYS){
        end = raycast(px, py, px+cos((pr+i)/180*PI)*LEN,
                      py+sin((pr+i)/180*PI)*LEN);
        if(fisheye_fix) end.len *= cos(i/180*PI);
        line(px*SCALE, py*SCALE, (px+cos((pr+i)/180*PI)*end.len)*SCALE,
             (py+sin((pr+i)/180*PI)*end.len)*SCALE, 0, 255, 0);
    }
#if SHOWPLAYER
    line(px*SCALE, py*SCALE, (px+cos(pr/180*PI))*SCALE,
         (py+sin(pr/180*PI))*SCALE, 255, 0, 0);
#endif
}

void render_world(void) {
    float i;
    int p;
    int c;
    float h;
    float l;
    RayEnd end;
    rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
    for(i=-(FOV/2),p=0;i<FOV/2;i+=FOV/(float)RAYS,p+=SCREEN_WIDTH/RAYS){
        end = raycast(px, py, px+cos((pr+i)/180*PI)*LEN,
                      py+sin((pr+i)/180*PI)*LEN);
        if(end.x_axis_hit){
            l = px+cos((pr+i)/180*PI)*end.len;
            l = l-floor(l);
            l *= TEX_WIDTH;
        }else{
            l = py+sin((pr+i)/180*PI)*end.len;
            l = l-floor(l);
            l *= TEX_WIDTH;
        }
        if(fisheye_fix) end.len *= cos(i/180*PI);
        h = SCREEN_HEIGHT/end.len;
        for(c=0;c<SCREEN_WIDTH/RAYS;c++){
#if TEXTURE
            
            texline(SCREEN_HEIGHT/2-h/2,
                    SCREEN_HEIGHT/2+h/2, p+c,
                    l);
#else
            if(h > SCREEN_HEIGHT) h = SCREEN_HEIGHT;
            vline(SCREEN_HEIGHT/2-h/2,
                  SCREEN_HEIGHT/2+h/2, p+c,
                  (255-(end.len/LEN*255))*(!end.x_axis_hit),
                  (255-(end.len/LEN*255))*end.x_axis_hit, 0);
#endif
        }
    }
}

RayEnd raycast(float x1, float y1, float x2, float y2) {
    int px = x1;
    int py = y1;
    float tmp;
    RayEnd end;
    Vector2 rays;
    Vector2 raystep;
    Vector2 steplen;
    Vector2 start;
    tmp = fabs(x2-x1);
    if(tmp) raystep.x = (y2-y1)/tmp;
    else raystep.x = (y2-y1);
    steplen.x = sqrt(raystep.x*raystep.x+1);
    tmp = fabs(y2-y1);
    if(tmp) raystep.y = (x2-x1)/tmp;
    else raystep.y = (x2-x1);
    steplen.y = sqrt(raystep.y*raystep.y+1);
    end.hit = 0;
    /* Calculate start */
    if(x1 < x2){
        start.x = 1-(x1-floor(x1));
    }else{
        start.x = x1-floor(x1);
    }
    if(y1 < y2){
        start.y = 1-(y1-floor(y1));
    }else{
        start.y = y1-floor(y1);
    }
    /* --------------- */
    rays.y = steplen.y*start.y;
    rays.x = steplen.x*start.x;
    if(rays.x < rays.y){
        end.len = rays.x;
        end.x_axis_hit = 1;
    }else{
        end.len = rays.y;
        end.x_axis_hit = 0;
    }
    while(end.len < LEN){
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
    float oldx;
    float oldy;
    printf("FPS: %d    \r", fps);
    fflush(stdout);
    if(render_keydown(&renderer, KEY_LEFT)) pr -= ROTSPEED/(float)fps;
    if(render_keydown(&renderer, KEY_RIGHT)) pr += ROTSPEED/(float)fps;
    if(render_keydown(&renderer, KEY_UP)){
        oldx = px;
        oldy = py;
        px += cos(pr/180*PI)*(SPEED/(float)fps);
        if(px >= 0 && px < MAP_WIDTH && py >= 0 && py < MAP_HEIGHT){
            if(map[(int)py*MAP_WIDTH+(int)px] != ' '){
                px = oldx;
            }
        }else{
            px = oldx;
        }
        py += sin(pr/180*PI)*(SPEED/(float)fps);
        if(px >= 0 && px < MAP_WIDTH && py >= 0 && py < MAP_HEIGHT){
            if(map[(int)py*MAP_WIDTH+(int)px] != ' '){
                py = oldy;
            }
        }else{
            py = oldy;
        }
    }
    if(render_keydown(&renderer, KEY_DOWN)){
        oldx = px;
        oldy = py;
        px -= cos(pr/180*PI)*(SPEED/(float)fps);
        if(px >= 0 && px < MAP_WIDTH && py >= 0 && py < MAP_HEIGHT){
            if(map[(int)py*MAP_WIDTH+(int)px] != ' '){
                px = oldx;
            }
        }else{
            px = oldx;
        }
        py -= sin(pr/180*PI)*(SPEED/(float)fps);
        if(px >= 0 && px < MAP_WIDTH && py >= 0 && py < MAP_HEIGHT){
            if(map[(int)py*MAP_WIDTH+(int)px] != ' '){
                py = oldy;
            }
        }else{
            py = oldy;
        }
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

