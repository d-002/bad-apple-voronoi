#include "cost.h"

#include <math.h>

#include "utils/utils.h"

double compute_cost(const struct image *image, struct voronoi_data *shared_data)
{
    // TODO only check around the edited cell if applicable
    size_t total_cost = 0;

    for (int y = 0; y < image->h; y += PRECISION)
        for (int x = 0; x < image->w; x += PRECISION)
        {
            enum color_class color1 = get_pixel(image, x, y);

            double closest_dist = -1;
            enum color_class color2 = false;
            for (int i = 0; i < N_CELLS; i++)
            {
                const struct cell *cell = shared_data->cells + i;
                double dist =
                    sqrt(SQR(cell->x - x) + SQR(cell->y - y)) * cell->weight;
                if (dist < closest_dist || closest_dist < 0)
                {
                    color2 = cell->color;
                    closest_dist = dist;
                }
            }

            total_cost += color1 != color2;
        }

    return (double)total_cost * SQR(PRECISION) / (image->w * image->h);
}

void compute_gradient(const struct image *image,
                      struct voronoi_data *shared_data, struct gradient *out)
{
    for (int i = 0; i < N_CELLS; i++)
    {
        struct cell *cell = shared_data->cells + i;
        const int x = cell->x, y = cell->y;
        const double w = cell->weight;
        const int x1 = MAX2(0, x - SAMPLE_POS_RADIUS),
                  y1 = MAX2(0, x - SAMPLE_POS_RADIUS);
        const double w1 = MAX2(MIN_WEIGHT, w - SAMPLE_WEIGHT_RADIUS);
        const int x2 = MIN2(image->w - 1, x + SAMPLE_POS_RADIUS),
                  y2 = MIN2(image->h, x + SAMPLE_POS_RADIUS);
        const double w2 = MIN2(MAX_WEIGHT, w + SAMPLE_WEIGHT_RADIUS);

        if (x1 == x2)
            out->dx[i] = 0;
        else
        {
            cell->x = x1;
            const double cost1 = compute_cost(image, shared_data);
            cell->x = x2;
            const double cost2 = compute_cost(image, shared_data);
            cell->x = x;
            out->dx[i] = (cost2 - cost1) / (x2 - x1);
        }
        if (y1 == y2)
            out->dy[i] = 0;
        else
        {
            cell->y = y1;
            const double cost1 = compute_cost(image, shared_data);
            cell->y = y2;
            const double cost2 = compute_cost(image, shared_data);
            cell->y = y;
            out->dy[i] = (cost2 - cost1) / (y2 - y1);
        }
        if (w1 == w2)
            out->dw[i] = 0;
        else
        {
            cell->weight = w1;
            const double cost1 = compute_cost(image, shared_data);
            cell->weight = w2;
            const double cost2 = compute_cost(image, shared_data);
            cell->weight = w;
            out->dw[i] = (cost2 - cost1) / (w2 - w1);
        }
    }
}
