#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>
#include <stdint.h>

#include "utils/errors.h"

struct image
{
    int w;
    int h;

    uint8_t *pixels;
};

// wrapper functions if I want to use Hilbert curves for better memory usage
static inline int get_pixel(struct image *image, int x, int y)
{
    size_t i = y * image->w + x;
    return image->pixels[i / 8] & (1 << (i % 8));
}

static inline void set_pixel(struct image *image, int x, int y, int value)
{
    size_t i = y * image->w + x;
    if (value)
        image->pixels[i / 8] |= 1 << (i % 8);
    else
        image->pixels[i / 8] &= ~(1 << (i % 8));
}

enum error_code image_load(const char *path, struct image *image);
enum error_code image_save(const struct image *image, const char *path);

#endif /* ! IMAGE_H */
