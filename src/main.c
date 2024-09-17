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
#include <raycaster.h>

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

Raycaster raycaster;
Renderer *renderer = &raycaster.renderer;

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

char lock = 0;

char map_view = 1;

char show_fps = 1;

void loop(int fps) {
    fixed_t oldx;
    fixed_t oldy;
    int tx, ty;
    if(render_keydown(renderer, KEY_LEFT)){
        raycaster.r -= TO_FIXED(ROTSPEED)/fps;
    }
    if(render_keydown(renderer, KEY_RIGHT)){
        raycaster.r += TO_FIXED(ROTSPEED)/fps;
    }
    if(render_keydown(renderer, KEY_UP)){
        oldx = raycaster.x;
        oldy = raycaster.y;
        raycaster.x += MUL(dcos(raycaster.r), DIV(TO_FIXED(SPEED),
                           TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(raycaster.x);
        ty = TO_INT(raycaster.y);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                raycaster.x = oldx;
            }
        }else{
            raycaster.x = oldx;
        }
#endif
        raycaster.y += MUL(dsin(raycaster.r), DIV(TO_FIXED(SPEED),
                           TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(raycaster.x);
        ty = TO_INT(raycaster.y);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                raycaster.y = oldy;
            }
        }else{
            raycaster.y = oldy;
        }
#endif
    }
    if(render_keydown(renderer, KEY_DOWN)){
        oldx = raycaster.x;
        oldy = raycaster.y;
        raycaster.x -= MUL(dcos(raycaster.r), DIV(TO_FIXED(SPEED),
                           TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(raycaster.x);
        ty = TO_INT(raycaster.y);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                raycaster.x = oldx;
            }
        }else{
            raycaster.x = oldx;
        }
#endif
        raycaster.y -= MUL(dsin(raycaster.r), DIV(TO_FIXED(SPEED),
                           TO_FIXED(fps)));
#if COLLISIONS
        tx = TO_INT(raycaster.x);
        ty = TO_INT(raycaster.y);
        if(tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT){
            if(map[ty*MAP_WIDTH+tx] != ' '){
                raycaster.y = oldy;
            }
        }else{
            raycaster.y = oldy;
        }
#endif
    }
    if(!lock){
        if(render_keydown(renderer, KEY_LCTRL)){
            map_view = !map_view;
            lock = 1;
        }
        if(render_keydown(renderer, KEY_SPACE)){
            raycaster.fisheye_fix = !raycaster.fisheye_fix;
            lock = 1;
        }
        if(render_keydown(renderer, KEY_LALT)){
            raycaster.texture = !raycaster.texture;
            lock = 1;
        }
        if(render_keydown(renderer, KEY_LSHIFT)){
            show_fps = !show_fps;
            lock = 1;
        }
    }else{
        lock = render_keydown(renderer, KEY_LCTRL) |
               render_keydown(renderer, KEY_SPACE) |
               render_keydown(renderer, KEY_LALT) |
               render_keydown(renderer, KEY_LSHIFT);
    }
    render_clear(renderer);
    if(map_view){
        raycaster_render_map(&raycaster);
    }else{
        raycaster_render_world(&raycaster);
    }
    if(show_fps) render_show_fps(renderer);
    render_update(renderer);
}

int main(int argc, char **argv) {
    raycaster_init(&raycaster, SCREEN_WIDTH, SCREEN_HEIGHT, "DDA Test", map,
                   MAP_WIDTH, MAP_HEIGHT, &wall, TO_FIXED(1.5), TO_FIXED(1.5),
                   TO_FIXED(45));
    render_main_loop(renderer, loop);
    return 0;
}

