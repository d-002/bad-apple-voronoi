#include "loadimage.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "logger/logger.h"

static void addCol(uint32_t col, float *red, float *green, float *blue,
                   float mult)
{
    *red += (col & 255) * mult;
    *green += (col >> 8 & 255) * mult;
    *blue += (col >> 16 & 255) * mult;
}

struct image loadResizedImage(char *path, int w, int h)
{
    SDL_Surface *_surf = IMG_Load(path);
    if (_surf == NULL)
    {
        logerror("Could not open image '%s'.", path);
        return IMAGE_ERROR;
    }

    SDL_Surface *surf =
        SDL_ConvertSurfaceFormat(_surf, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(_surf);

    if (surf == NULL)
    {
        logerror("Could not convert image '%s'.", path);
        return IMAGE_ERROR;
    }

    SDL_LockSurface(surf);

    uint32_t *pixels = malloc(w * h * sizeof(uint32_t));
    if (pixels == NULL)
    {
        SDL_UnlockSurface(surf);
        SDL_FreeSurface(surf);
        log_alloc_error();
        return IMAGE_ERROR;
    }

    uint32_t *surfPixels = surf->pixels;

    int sw = surf->w, sh = surf->h;
    float mult = (float)sw / sh < (float)w / h ? (float)sh / h : (float)sw / w;

    // fill buffer with centered, bilinearly scaled image

    // get the average colors of the image borders
    // in case padding is necessary
    uint32_t left = 0, right = 0, top = 0, bottom = 0;
    float multw = 1.0 / sw, multh = 1.0 / sh;

    float redA = 0, greenA = 0, blueA = 0, redB = 0, greenB = 0, blueB = 0;
    for (int y = 0; y < sh; y++)
    {
        addCol(surfPixels[y * sw], &redA, &greenA, &blueA, multh);
        addCol(surfPixels[y * sw + sw - 1], &redB, &greenB, &blueB, multh);
    }
    left = ((int)blueA << 16) + ((int)greenA << 8) + (int)redA;
    right = ((int)blueB << 16) + ((int)greenB << 8) + (int)redB;

    redA = 0, greenA = 0, blueA = 0, redB = 0, greenB = 0, blueB = 0;
    for (int x = 0; x < sw; x++)
    {
        addCol(surfPixels[x], &redA, &greenA, &blueA, multw);
        addCol(surfPixels[(sh - 1) * sw + x], &redB, &greenB, &blueB, multw);
    }
    top = ((int)blueA << 16) + ((int)greenA << 8) + (int)redA;
    bottom = ((int)blueB << 16) + ((int)greenB << 8) + (int)redB;

    int paddingX = (int)((w - 1) * mult - sw + 1) >> 1;
    int paddingY = (int)((h - 1) * mult - sh + 1) >> 1;

    for (int x = 0; x < w; x++)
    {
        float fx = x * mult - paddingX;

        int qx = fx;
        int okX = qx < sw - 1;
        float rx = okX ? fx - qx : 0;
        float rx1 = 1 - rx;

        for (int y = 0; y < h; y++)
        {
            float fy = y * mult - paddingY;

            int qy = fy;
            int okY = qy < sh - 1;
            float ry = okY ? fy - qy : 0;
            float ry1 = 1 - ry;

            float col = 0;
            if (fx < 0)
                col = left;
            else if (fx >= sw)
                col = right;
            else if (fy < 0)
                col = top;
            else if (fy >= sh)
                col = bottom;
            else
            {
                // bilinear filtering

                float red = 0, green = 0, blue = 0;
                addCol(surfPixels[qy * sw + qx], &red, &green, &blue,
                       rx1 * ry1);

                if (okX)
                {
                    if (okY)
                        addCol(surfPixels[(qy + 1) * sw + qx + 1], &red, &green,
                               &blue, rx * ry);
                    addCol(surfPixels[qy * sw + qx + 1], &red, &green, &blue,
                           rx * ry1);
                }
                if (okY)
                    addCol(surfPixels[(qy + 1) * sw + qx], &red, &green, &blue,
                           rx1 * ry);

                col = ((uint8_t)blue << 16) + ((uint8_t)green << 8)
                    + (uint8_t)red;
            }

            pixels[y * w + x] = col;
        }
    }

    SDL_UnlockSurface(surf);
    SDL_FreeSurface(surf);
    return (struct image){ .w = w, .h = h, .pixels = pixels };
}
