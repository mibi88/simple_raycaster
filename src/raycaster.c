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

#include <raycaster.h>

#define RENDERER r->renderer

void raycaster_init(Raycaster *r, int width, int height, char *title,
                    char *map, int map_width, int map_height, Texture *tex,
                    fixed_t x, fixed_t y, fixed_t a) {
    render_init(&RENDERER, width, height, title);
    /* Settings */
    r->fov = 60;
    r->rays = width;
    r->scale = width/map_width;
    if(height/map_height < r->scale) r->scale = height/map_height;
    r->len = 25;
    r->speed = 5;
    r->rotspeed = 100;
    r->width = width;
    r->height = height;
    /* Features */
    r->texture = 1;
    r->fisheye_fix = 1;
    /* Data */
    r->tex = tex;
    r->map = map;
    r->map_width = map_width;
    r->map_height = map_height;
    /* View */
    r->x = x;
    r->y = y;
    r->r = a;
}

Texture *_get_tile_tex(Raycaster *r, int cx, int cy) {
    return r->tex;
}

void raycaster_render_map(Raycaster *r) {
    int x, y;
    fixed_t i;
    RayEnd end;
    for(y=0;y<r->map_height;y++){
        for(x=0;x<r->map_width;x++){
            if(r->map[y*r->map_width+x] != ' '){
                render_rect(&RENDERER, x*r->scale, y*r->scale, r->scale,
                            r->scale, 0, 0, 0);
            }
        }
    }
    for(i=-(TO_FIXED(r->fov)/2);i<TO_FIXED(r->fov)/2;
        i+=TO_FIXED(r->fov)/r->rays){
        end = raycaster_raycast(r, r->x, r->y, r->x+dcos(r->r+i)*r->len,
                                r->y+dsin(r->r+i)*r->len);
        if(r->fisheye_fix) end.len = MUL(end.len, dcos(i));
        render_line(&RENDERER, TO_INT(r->x*r->scale), TO_INT(r->y*r->scale),
                    TO_INT((r->x+MUL(dcos(r->r+i), end.len))*r->scale),
                    TO_INT((r->y+MUL(dsin(r->r+i), end.len))*r->scale), 0, 255,
                    0);
    }
    render_line(&RENDERER, TO_INT(r->x*r->scale), TO_INT(r->y*r->scale),
                TO_INT((r->x+dcos(r->r))*r->scale),
                TO_INT((r->y+dsin(r->r))*r->scale), 255, 0, 0);
}

void raycaster_render_world(Raycaster *r) {
    fixed_t i;
    int p;
    int c;
    int h;
    fixed_t l;
    int no_clip_h;
    RayEnd end;
    Texture *tex;
    for(i=-(TO_FIXED(r->fov)/2),p=0;i<TO_FIXED(r->fov)/2;
        i+=TO_FIXED(r->fov)/r->rays, p+=r->width/r->rays){
        end = raycaster_raycast(r, r->x, r->y, r->x+dcos(r->r+i)*r->len,
                                r->y+dsin(r->r+i)*r->len);
        if(!end.hit){
            for(c=0;c<r->width/r->rays;c++){
                render_vline(&RENDERER, 0, r->height, p+c, 0, 0, 0);
            }
            continue;
        }
        tex = _get_tile_tex(r, end.cx, end.cy);
        if(end.x_axis_hit){
            l = r->x+MUL(dcos(r->r+i), end.len);
            l = l-FLOOR(l);
            l *= tex->width;
        }else{
            l = r->y+MUL(dsin(r->r+i), end.len);
            l = l-FLOOR(l);
            l *= tex->width;
        }
        if(r->fisheye_fix) end.len = MUL(end.len, dcos(i));
        h = TO_INT(DIV(TO_FIXED(r->height), (end.len ? end.len : 1)));
        no_clip_h = h;
        if(h > r->height) h = r->height;
        for(c=0;c<r->width/r->rays;c++){
            render_vline(&RENDERER, 0, r->height/2-h/2, p+c, 0, 0, 0);
            render_vline(&RENDERER, r->height/2+h/2, r->height, p+c, 0,
                         0, 0);
            if(r->texture){
            render_texvline(&RENDERER, tex, r->height/2-h/2,
                            r->height/2+h/2, r->height/2-no_clip_h/2,
                            r->height/2+no_clip_h/2, p+c, TO_INT(l),
                            255-TO_INT(end.len/r->len*255));
            }else{
                render_vline(&RENDERER, r->height/2-h/2,
                             r->height/2+h/2, p+c,
                             (255-TO_INT(end.len/r->len*255))*(!end.x_axis_hit),
                             (255-TO_INT(end.len/r->len*255))*end.x_axis_hit,
                             0);
            }
        }
    }
}

RayEnd raycaster_raycast(Raycaster *r, fixed_t x1, fixed_t y1, fixed_t x2,
                         fixed_t y2) {
    int px = TO_INT(x1);
    int py = TO_INT(y1);
    fixed_t tmp;
    RayEnd end;
    Vector2 rays;
    Vector2 raystep;
    Vector2 steplen;
    Vector2 start;
    tmp = ABS(x2-x1);
    if(!tmp) tmp++;
    raystep.x = DIV((y2-y1), tmp);
    steplen.x = fsqrt(MUL(raystep.x, raystep.x)+TO_FIXED(1));
    tmp = ABS(y2-y1);
    if(!tmp) tmp++;
    raystep.y = DIV((x2-x1), tmp);
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
    while(end.len < TO_FIXED(r->len)){
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
        if(px >= 0 && px < r->map_width && py >= 0 && py < r->map_height){
            end.cx = px;
            end.cy = py;
            if(r->map[py*r->map_width+px] != ' '){
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
