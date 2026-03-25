#ifndef COST_H
#define COST_H

#include "image/image.h"
#include "shared_data.h"

// how much to downsample the image
#define PRECISION 4
// sample distance for gradient estimation
#define SAMPLE_POS_RADIUS 8
#define SAMPLE_WEIGHT_RADIUS .05
#define SAMPLE_COLOR_RADIUS .01
// weights bounds to keep them from diverging
#define MIN_WEIGHT 1
#define MAX_WEIGHT 3

struct gradient
{
    double dx[N_CELLS];
    double dy[N_CELLS];
    double dw[N_CELLS];
    double dc[N_CELLS];
};

double compute_cost(const struct image *image,
                    struct voronoi_data *shared_data);
void compute_gradient(const struct image *image,
                      struct voronoi_data *shared_data, struct gradient *out,
                      double cost1);

#endif /* ! COST_H */
