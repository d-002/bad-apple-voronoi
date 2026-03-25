#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "utils/errors.h"
#include "voronoi/cell.h"

struct image
{
    int w;
    int h;

    uint8_t *pixels;
};

// wrapper functions if I want to use Hilbert curves for better memory usage
static inline enum color_class get_pixel(const struct image *image, const int x,
                                         const int y)
{
    size_t i = y * image->w + x;
    return image->pixels[i / 8] & (1 << (i % 8));
}

static inline void set_pixel(struct image *image, const int x, const int y,
                             const enum color_class value)
{
    size_t i = y * image->w + x;
    if (value)
        image->pixels[i / 8] = 255;
    else
        image->pixels[i / 8] = 0;
}

enum error_code image_load(const char *path, struct image *image);
enum error_code image_save(const struct image *image, const char *path);

#endif /* ! IMAGE_H */
