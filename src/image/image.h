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
    size_t size; // cached

    uint8_t *pixels;
};

// wrapper functions if I want to use Hilbert curves for better memory usage
static inline enum color_class get_pixel(const struct image *image, const int x,
                                         const int y)
{
    const size_t i = (size_t)y * image->w + x;
    const size_t byte_index = i / 8;
    const size_t bit_index = i % 8;
    return (image->pixels[byte_index] & (1 << (7 - bit_index))) == 0 ? BLACK
                                                                     : WHITE;
}

static inline void set_pixel(struct image *image, const int x, const int y,
                             const enum color_class value)
{
    const size_t i = (size_t)y * image->w + x;
    const size_t byte_index = i / 8;
    const size_t bit_index = i % 8;
    if (value)
        image->pixels[byte_index] |= 1 << (7 - bit_index);
    else
        image->pixels[byte_index] &= ~(1 << (7 - bit_index));
}

enum error_code image_load(const char *path, struct image *image);
enum error_code image_save(const struct image *image, const char *path);

#endif /* ! IMAGE_H */
