#ifndef COST_H
#define COST_H

#include "image/image.h"
#include "shared_data.h"

#define PRECISION 6
#define SAMPLE_POS_RADIUS PRECISION
#define SAMPLE_WEIGHT_RADIUS .02
#define MIN_WEIGHT .1
#define MAX_WEIGHT 3

struct gradient
{
    double dx[N_CELLS];
    double dy[N_CELLS];
    double dw[N_CELLS];
};

double compute_cost(const struct image *image,
                    struct voronoi_data *shared_data);
void compute_gradient(const struct image *image,
                      struct voronoi_data *shared_data, struct gradient *out);

#endif /* ! COST_H */
