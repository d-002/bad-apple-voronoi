#include "apply.h"

#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"
#include "utils/utils.h"

enum error_code image_apply_voronoi(struct image *image,
                                    const struct voronoi_data *shared_data)
{
    int *indices = malloc(image->size * sizeof(int));
    if (indices == NULL)
    {
        log_alloc_error();
        return ALLOC_ERROR;
    }

    for (int y = 0; y < image->h; y++)
        for (int x = 0; x < image->w; x++)
        {
            double closest_dist = -1;
            int closest_i;
            enum color_class color = BLACK;
            for (int i = 0; i < N_CELLS; i++)
            {
                const struct cell *cell = shared_data->cells + i;
                double dist = SQR(cell->x - x) + SQR(cell->y - y);
#ifdef WEIGHTED
                dist *= SQR(cell->weight);
#endif /* WEIGHTED */
                if (dist < closest_dist || closest_dist < 0)
                {
                    color = cell->color;
                    closest_dist = dist;
                    closest_i = i;
                }
            }

            // swap the color around the sites to make them apparent
            if (closest_dist < 3)
                color = color == BLACK ? WHITE : BLACK;

            set_pixel(image, x, y, color);
            indices[y * image->w + x] = closest_i;
        }

    // swap the colors around the edges
    for (int y = 0; y < image->h - 1; y++)
        for (int x = 0; x < image->w - 1; x++)
        {
            int i00 = indices[y * image->w + x];
            int i01 = indices[y * image->w + (x + 1)];
            int i10 = indices[(y + 1) * image->w + x];
            int i11 = indices[(y + 1) * image->w + (x + 1)];
            enum color_class c00 = shared_data->cells[i00].color;
            enum color_class c01 = shared_data->cells[i01].color;
            enum color_class c10 = shared_data->cells[i10].color;
            enum color_class c11 = shared_data->cells[i11].color;
            if ((i00 == i01 && i00 == i10 && i00 == i11 && i01 == i10
                 && i01 == i11 && i10 == i11)
                || (c00 != c01 || c00 != c10 || c00 != c11 || c01 != c10
                    || c01 != c11 || c10 != c11))
                continue;

            set_pixel(image, x, y,
                      get_pixel(image, x, y) == WHITE ? BLACK : WHITE);
        }
    free(indices);

    return SUCCESS;
}
