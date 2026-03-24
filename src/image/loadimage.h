#ifndef LOAD_IMAGE_H
#define LOAD_IMAGE_H

#include <stdint.h>

struct image
{
    int w;
    int h;
    uint32_t *pixels;
};

#define IMAGE_ERROR                                                            \
    (struct image)                                                             \
    {                                                                          \
        .w = 0, .h = 0, .pixels = NULL                                         \
    }

struct image loadResizedImage(char *path, int w, int h);

#endif /* ! LOAD_IMAGE_H */
