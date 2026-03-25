#include "cost.h"

#include <math.h>

#include "utils/utils.h"

double compute_cost(const struct image *image, struct voronoi_data *shared_data)
{
    // TODO only check around the edited cell if applicable
    double total_cost = 0;

    for (int y = 0; y < image->h; y += PRECISION)
        for (int x = 0; x < image->w; x += PRECISION)
        {
            enum color_class color1 = get_pixel(image, x, y);

            double closest_dist = -1;
            double color2 = .5;
            for (int i = 0; i < N_CELLS; i++)
            {
                const struct cell *cell = shared_data->cells + i;
                double dist = sqrt(SQR(cell->x - x) + SQR(cell->y - y));
#ifdef WEIGHTED
                dist *= cell->weight;
#endif /* WEIGHTED */
                if (dist < closest_dist || closest_dist < 0)
                {
                    color2 = cell->training_color;
                    closest_dist = dist;
                }
            }

            total_cost += ABS(color1 - color2);
        }

    return total_cost * SQR(PRECISION) / (image->w * image->h);
}

void compute_gradient(const struct image *image,
                      struct voronoi_data *shared_data, struct gradient *out,
                      double cost1)
{
    for (int i = 0; i < N_CELLS; i++)
    {
        struct cell *cell = shared_data->cells + i;
        const double x = cell->x, y = cell->y;
        const double c = cell->training_color;
#ifdef WEIGHTED
        const double w = cell->weight;
#endif /* WEIGHTED */

        // find points in bounds to avoid using e.g. x2-x conditions
        const double x2 = x < image->w / 2. ? x + SAMPLE_POS_RADIUS
                                            : x - SAMPLE_POS_RADIUS,
                     y2 = y < image->h / 2. ? y + SAMPLE_POS_RADIUS
                                            : y - SAMPLE_POS_RADIUS;
        const double c2 =
            c < .5 ? c + SAMPLE_COLOR_RADIUS : c - SAMPLE_COLOR_RADIUS;
#ifdef WEIGHTED
        const double w2 = w < (MIN_WEIGHT + MAX_WEIGHT) / 2.
            ? w + SAMPLE_WEIGHT_RADIUS
            : w - SAMPLE_WEIGHT_RADIUS;
#endif /* WEIGHTED */

        cell->x = x2;
        double cost2 = compute_cost(image, shared_data);
        cell->x = x;
        out->dx[i] = (cost2 - cost1) / (x2 - x);

        cell->y = y2;
        cost2 = compute_cost(image, shared_data);
        cell->y = y;
        out->dy[i] = (cost2 - cost1) / (y2 - y);

#ifdef WEIGHTED
        cell->weight = w2;
        cost2 = compute_cost(image, shared_data);
        cell->weight = w;
        out->dw[i] = (cost2 - cost1) / (w2 - w);
#endif /* WEIGHTED */

        cell->training_color = c2;
        cost2 = compute_cost(image, shared_data);
        cell->training_color = c;
        out->dc[i] = (cost2 - cost1) / (c2 - c);
    }
}
