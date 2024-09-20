#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct {
    const unsigned int *data;
    int width;
    int height;
    void *extradata;
} Texture;

#define TEX_WIDTH(tex) ((tex)->width)
#define TEX_HEIGHT(tex) ((tex)->height)

#endif
