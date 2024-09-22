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

#include <stdlib.h>

#define RENDERER r->renderer

void raycaster_init(Raycaster *r, int width, int height, char *title,
                    Map *map, fixed_t x, fixed_t y, fixed_t a,
                    fixed_t *zbuffer) {
    linit();
    render_init(&RENDERER, width, height, title);
    r->width = render_get_width(&RENDERER);
    r->height = render_get_height(&RENDERER);
    /* Settings */
    r->fov = 60;
    r->rays = r->width;
    r->scale = r->width/map->width;
    if(r->height/map->height < r->scale) r->scale = r->height/map->height;
    r->len = 25;
    r->speed = 5;
    r->rotspeed = 100;
    /* Features */
    r->texture = 1;
    r->fisheye_fix = 1;
    /* Data */
    r->map = map;
    r->map_width = map->width;
    r->map_height = map->height;
    r->zbuffer = zbuffer;
    r->sprite_num = map->sprite_num;
    r->sprites = map->sprites;
    /* View */
    r->x = x;
    r->y = y;
    r->r = a;
}

void raycaster_set_sprites(Raycaster *r, Sprite *sprites, int sprite_num) {
    r->sprites = sprites;
    r->sprite_num = sprite_num;
}

Texture *_get_tile_tex(Raycaster *r, int cx, int cy) {
    if(cx >= 0 && cx < r->map_width && cy >= 0 && cy < r->map_height){
        return r->map->tileset[r->map->data[cy*r->map_width+cx]-1].texture;
    }
    return r->map->tileset[0].texture;
}

void raycaster_render_map(Raycaster *r) {
    int x, y;
    fixed_t i;
    RayEnd end;
    render_clear(&RENDERER, 0);
    for(y=0;y<r->map_height;y++){
        for(x=0;x<r->map_width;x++){
            if(r->map->data[y*r->map_width+x]){
                render_rect(&RENDERER, x*r->scale, y*r->scale, r->scale,
                            r->scale, 0, 0, 0);
            }
        }
    }
    for(i=-(TO_FIXED(r->fov)/2);i<TO_FIXED(r->fov)/2;
        i+=TO_FIXED(r->fov)/r->rays){
        end = raycaster_raycast(r, r->x, r->y, r->x+DCOS(r->r+i)*r->len,
                                r->y+DSIN(r->r+i)*r->len);
        if(r->fisheye_fix) end.len = MUL(end.len, DCOS(i));
        render_line(&RENDERER, TO_INT(r->x*r->scale), TO_INT(r->y*r->scale),
                    TO_INT((r->x+MUL(DCOS(r->r+i), end.len))*r->scale),
                    TO_INT((r->y+MUL(DSIN(r->r+i), end.len))*r->scale), 0, 255,
                    0);
    }
    for(x=0;x<r->sprite_num;x++){
        render_set_pixel(&RENDERER, TO_INT(r->sprites[x].x*r->scale),
                         TO_INT(r->sprites[x].y*r->scale), 0, 0, 255);
    }
    render_line(&RENDERER, TO_INT(r->x*r->scale), TO_INT(r->y*r->scale),
                TO_INT((r->x+DCOS(r->r))*r->scale),
                TO_INT((r->y+DSIN(r->r))*r->scale), 255, 0, 0);
}

int _raycaster_sort_sprites(const void *item1, const void *item2) {
    Sprite *sprite1 = (Sprite*)item1;
    Sprite *sprite2 = (Sprite*)item2;
    if(sprite1->dist < sprite2->dist) return 1;
    if(sprite1->dist == sprite2->dist) return 0;
    else return -1;
}

void raycaster_render_world(Raycaster *r) {
    fixed_t i;
    int p;
    int c;
    int h;
    Sprite *sprite;
    fixed_t a;
    fixed_t tmp;
    int x;
    fixed_t inc;
    fixed_t l;
    int no_clip_h;
    int t;
    RayEnd end;
    Texture *tex;
    while(r->r < 0) r->r += TO_FIXED(360);
    while(r->r > TO_FIXED(360)) r->r -= TO_FIXED(360);
#if !NOCLEAR
    render_clear(&RENDERER, 1);
#endif
    for(i=-(TO_FIXED(r->fov)/2),p=0;i<TO_FIXED(r->fov)/2;
        i+=TO_FIXED(r->fov)/r->rays, p+=r->width/r->rays){
        end = raycaster_raycast(r, r->x, r->y, r->x+DCOS(r->r+i)*r->len,
                                r->y+DSIN(r->r+i)*r->len);
        r->zbuffer[p] = end.len;
        if(!end.hit){
            for(c=0;c<r->width/r->rays;c++){
                render_vline(&RENDERER, 0, r->height, p+c, 0, 0, 0);
            }
            continue;
        }
        tex = _get_tile_tex(r, end.cx, end.cy);
        if(end.x_axis_hit){
            l = r->x+MUL(DCOS(r->r+i), end.len);
            l = l-FLOOR(l);
            l *= TEX_WIDTH(tex);
        }else{
            l = r->y+MUL(DSIN(r->r+i), end.len);
            l = l-FLOOR(l);
            l *= TEX_WIDTH(tex);
        }
        if(r->fisheye_fix) end.len = MUL(end.len, DCOS(i));
        h = TO_INT(DIV(TO_FIXED(r->height), (end.len ? end.len : 1)));
        no_clip_h = h;
        if(h > r->height) h = r->height;
        for(c=0;c<r->width/r->rays;c++){
#if NOCLEAR
            render_vline(&RENDERER, 0, r->height/2-h/2, p+c, 0, 0, 0);
            render_vline(&RENDERER, r->height/2+h/2, r->height, p+c, 0,
                         0, 0);
#endif
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
    if(r->sprite_num > 0){
        for(p=0;p<r->sprite_num;p++){
            sprite = r->sprites+p;
            sprite->dist = SQRT(MUL(r->x-sprite->x, r->x-sprite->x)+
                                MUL(r->y-sprite->y, r->y-sprite->y));
        }
        if(r->sprite_num > 1){
            qsort(r->sprites, r->sprite_num, sizeof(Sprite),
                  _raycaster_sort_sprites);
        }
        for(p=0;p<r->sprite_num;p++){
            sprite = r->sprites+p;
            if(sprite->dist > TO_FIXED(r->len) || !sprite->visible){
                sprite->screen_x = -1;
                sprite->h = 0;
                continue;
            }
            /* Calculate the position of the sprite on screen. */
            a = datan2(sprite->y-r->y, sprite->x-r->x);
            if(a < 0) a += TO_FIXED(360);
            tmp = r->r+TO_FIXED(r->fov)/2-a;
            tmp = tmp;
            if(a > TO_FIXED(270) && r->r < TO_FIXED(90)) tmp += TO_FIXED(360);
            if(r->r > TO_FIXED(270) && a < TO_FIXED(90)) tmp -= TO_FIXED(360);
            x = r->width-TO_INT(tmp/r->fov*r->width);
            sprite->screen_x = x;
            h = TO_INT(DIV(TO_FIXED(r->height),
                           (sprite->dist ? sprite->dist : 1)));
            no_clip_h = h;
            sprite->h = h;
            if(h > r->height) h = r->height;
            inc = TO_FIXED(TEX_WIDTH(sprite->texture))/no_clip_h;
            if(x+no_clip_h/2 >= 0 && x-no_clip_h/2 < r->width){
                for(t=0,i=x-no_clip_h/2;i<x+no_clip_h/2;i++,t++){
                    if(i >= 0 && i < r->width){
                        if(r->zbuffer[i] > sprite->dist){
                            render_texvline(&RENDERER, sprite->texture,
                                            r->height/2-h/2, r->height/2+h/2,
                                            r->height/2-no_clip_h/2,
                                            r->height/2+no_clip_h/2, i,
                                            TO_INT(t*inc),
                                            (255-TO_INT(sprite->dist/r->len*
                                             255)));
                        }
                    }
                }
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
    steplen.x = SQRT(MUL(raystep.x, raystep.x)+TO_FIXED(1));
    tmp = ABS(y2-y1);
    if(!tmp) tmp++;
    raystep.y = DIV((x2-x1), tmp);
    steplen.y = SQRT(MUL(raystep.y, raystep.y)+TO_FIXED(1));
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
            if(r->map->data[py*r->map_width+px]){
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
