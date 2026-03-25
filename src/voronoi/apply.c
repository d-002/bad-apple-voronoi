#include "apply.h"

#include <math.h>

#include "utils/utils.h"

enum error_code image_apply_voronoi(struct image *image,
                                    const struct voronoi_data *shared_data)
{
    for (int y = 0; y < image->h; y++)
        for (int x = 0; x < image->w; x++)
        {
            double closest_dist = -1;
            enum color_class color = BLACK;
            for (int i = 0; i < N_CELLS; i++)
            {
                const struct cell *cell = shared_data->cells + i;
                double dist =
                    sqrt(SQR(cell->x - x) + SQR(cell->y - y)) * cell->weight;
                if (dist < closest_dist || closest_dist < 0)
                {
                    color = cell->color;
                    closest_dist = dist;
                }
            }

            // swap the color around the sites to make them apparent
            if (closest_dist < 3)
                color = color == BLACK ? WHITE : BLACK;

            set_pixel(image, x, y, color);
        }

    return SUCCESS;
}
