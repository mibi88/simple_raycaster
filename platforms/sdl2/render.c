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
#include <SDL2/SDL.h>

#include <fixed.h>

void render_init(Renderer *renderer, int width, int height, char *title) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        fputs("[render] Failed to initialize the SDL2!", stderr);
        exit(-1);
    }
    renderer->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, width, height,
                                        SDL_WINDOW_SHOWN |
                                        SDL_WINDOW_RESIZABLE);
    if(!renderer->window){
        fputs("[render] Failed to create the SDL2 window!", stderr);
        exit(-1);
    }
    renderer->renderer = SDL_CreateRenderer(renderer->window, -1, 0);
    if(!renderer->renderer){
        fputs("[render] Failed to create a renderer for the SDL2 window!",
              stderr);
        SDL_DestroyWindow(renderer->window);
        exit(-1);
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    renderer->w = width;
    renderer->h = height;
    SDL_MaximizeWindow(renderer->window);
    SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
    render_clear(renderer, 0);
}

void render_set_pixel(Renderer *renderer, int x, int y, int r, int g, int b) {
    if(x >= 0 && x < renderer->w && y >= 0 && y < renderer->h){
        SDL_SetRenderDrawColor(renderer->renderer, r, g, b, 255);
        SDL_RenderDrawPoint(renderer->renderer, x, y);
    }
}

void render_line(Renderer *renderer, int x1, int y1, int x2, int y2, int r,
                 int g, int b) {
    SDL_SetRenderDrawColor(renderer->renderer, r, g, b, 255);
    SDL_RenderDrawLine(renderer->renderer, x1, y1, x2, y2);
}

void render_rect(Renderer *renderer, int sx, int sy, int w, int h, int r,
                 int g, int b) {
    SDL_Rect rect;
    rect.x = sx;
    rect.y = sy;
    rect.w = w;
    rect.h = h;
    SDL_SetRenderDrawColor(renderer->renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer->renderer, &rect);
}

void render_vline(Renderer *renderer, int y1, int y2, int x, int r, int g,
                  int b) {
    int y;
    if(x < 0 || x >= renderer->w) return;
    if(y1 < 0) y1 = 0;
    else if(y1 >= renderer->h) y1 = renderer->h-1;
    if(y2 >= renderer->h) y2 = renderer->h-1;
    else if(y2 < 0) y2 = 0;
    SDL_SetRenderDrawColor(renderer->renderer, r, g, b, 255);
    SDL_RenderDrawLine(renderer->renderer, x, y1, x, y2);
}

void render_texvline(Renderer *renderer, Texture *tex, int y1, int y2, int ty1,
                     int ty2, int x, int l, int fog) {
#if FAST_TEXTURING
    SDL_Rect texrect;
    SDL_Rect destrect;
    if(!tex->extradata){
        /* Create an SDL texture. */
        tex->extradata = SDL_CreateTexture(renderer->renderer,
                                           SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           tex->width, tex->height);
        if(!tex->extradata){
            return;
        }
        if(SDL_UpdateTexture(tex->extradata, NULL, tex->data, tex->width*4)){
            return;
        }
        SDL_SetTextureBlendMode(tex->extradata, SDL_BLENDMODE_BLEND);
    }
    if(l < 0 || l >= tex->width) return;
    texrect.x = l;
    texrect.y = 0;
    texrect.w = 1;
    texrect.h = tex->height;
    destrect.x = x;
    destrect.y = ty1 < y2 ? ty1 : ty2;
    destrect.w = 1;
    destrect.h = ABS(ty2-ty1);
    SDL_SetTextureColorMod(tex->extradata, fog, fog, fog);
    SDL_RenderCopyEx(renderer->renderer, tex->extradata, &texrect, &destrect,
                     0, NULL, SDL_FLIP_NONE);
#else
    int y;
    int r, g, b;
    int p;
    int n;
    int t;
    unsigned int h = ABS(ty2-ty1);
    ufixed_t texinc = UTO_FIXED(tex->height)/(h ? h : 1);
    if(x < 0 || x >= renderer->w) return;
    if(y1 < 0) y1 = 0;
    else if(y1 >= renderer->h) y1 = renderer->h-1;
    if(y2 >= renderer->h) y2 = renderer->h-1;
    else if(y2 < 0) y2 = 0;
    if(l >= tex->width) l = tex->width-1;
    else if(l < 0) l = 0;
    for(t=y1-ty1,n=0,y=y1;y<y2;y+=y1<y2 ? 1 : -1,n++,t++){
        p = UTO_INT(texinc*t);
        if(p < 0) p = 0;
        else if(p >= tex->height) p = tex->height-1;
        r = tex->data[p*tex->width+l]>>24;
        g = (tex->data[p*tex->width+l]>>16)&0xFF;
        b = (tex->data[p*tex->width+l]>>8)&0xFF;
        r = TO_INT(TO_FIXED(r)/255*fog);
        g = TO_INT(TO_FIXED(g)/255*fog);
        b = TO_INT(TO_FIXED(b)/255*fog);
        SDL_SetRenderDrawColor(renderer->renderer, r, g, b, 255);
        SDL_RenderDrawPoint(renderer->renderer, x, y);
    }
#endif
}

void render_update(Renderer *renderer) {
    SDL_RenderFlush(renderer->renderer);
    SDL_RenderPresent(renderer->renderer);
}

void render_clear(Renderer *renderer, char black) {
    SDL_SetRenderDrawColor(renderer->renderer, black ? 0x00 : 0xFF,
                           black ? 0x00 : 0xFF, black ? 0x00 : 0xFF,
                           SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer->renderer);
}

char render_keydown(Renderer *renderer, int key) {
    Uint8 *keybuffer;
    const int keymap[KEY_AMOUNT] = {
        SDL_SCANCODE_UP,
        SDL_SCANCODE_DOWN,
        SDL_SCANCODE_LEFT,
        SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_SPACE,
        SDL_SCANCODE_LCTRL,
        SDL_SCANCODE_LALT,
        SDL_SCANCODE_LSHIFT
    };
    SDL_PumpEvents();
    keybuffer = (Uint8*)SDL_GetKeyboardState(NULL);
    if(key >= 0 && key < KEY_AMOUNT){
        return keybuffer[keymap[key]];
    }
    return 0;
}

int render_get_width(Renderer *renderer) {
    return renderer->w;
}

int render_get_height(Renderer *renderer) {
    return renderer->h;
}

int render_ms(Renderer *renderer) {
    return SDL_GetTicks();
}

void render_show_fps(Renderer *renderer) {
    /* TODO: Show the FPS in the window */
    printf("FPS: %d    \r", renderer->fps);
    fflush(stdout);
}

void render_main_loop(Renderer *renderer, void (*loop_function)(int)) {
    SDL_Event event;
    int w, h;
    float xscale, yscale;
    Uint32 _last_t;
    int time;
    for(;;){
        /* Handle events */
        while(SDL_PollEvent(&event)){
            if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                puts("[render] The window size changed!");
                SDL_GetRendererOutputSize(renderer->renderer ,&w, &h);
                xscale = (float)w/(float)renderer->w;
                yscale = (float)h/(float)renderer->h;
                if(xscale < yscale) yscale = xscale;
                else xscale = yscale;
                if(SDL_RenderSetScale(renderer->renderer, xscale, yscale)){
                    puts("[render] Failed to scale renderer!");
                }
            }
            if(event.type == SDL_QUIT){
                break;
            }
        }
        if(event.type == SDL_QUIT){
            break;
        }
        /* Handle time and call loop_function */
        _last_t = SDL_GetTicks();
        loop_function(renderer->fps ? renderer->fps : 1);
        while(SDL_GetTicks() - _last_t < 20);
        time = SDL_GetTicks() - _last_t;
        time = time ? time : 1;
        renderer->fps = 1000/time;
    }
    SDL_DestroyRenderer(renderer->renderer);
    SDL_DestroyWindow(renderer->window);
    SDL_Quit();
}
