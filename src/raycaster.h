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

#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <fixed.h>
#include <render.h>
#include <texture.h>
#include <map.h>

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

typedef struct {
    /* Settings */
    int fov;
    int rays;
    int scale;
    int len;
    int speed;
    int rotspeed;
    int width;
    int height;
    /* Features */
    char texture;
    char fisheye_fix;
    /* Data */
    fixed_t *zbuffer;
    Map *map;
    int map_width;
    int map_height;
    Sprite *sprites;
    int sprite_num;
    /* View */
    fixed_t x;
    fixed_t y;
    fixed_t r;
    Renderer renderer;
} Raycaster;

void raycaster_init(Raycaster *r, int width, int height, char *title,
                    Map *map, fixed_t x, fixed_t y, fixed_t a,
                    fixed_t *zbuffer);

void raycaster_set_sprites(Raycaster *r, Sprite *sprites, int sprite_num);

void raycaster_render_map(Raycaster *r);

void raycaster_render_world(Raycaster *r);

RayEnd raycaster_raycast(Raycaster *r, fixed_t x1, fixed_t y1, fixed_t x2,
                         fixed_t y2);

void raycaster_render_map(Raycaster *r);



#endif
