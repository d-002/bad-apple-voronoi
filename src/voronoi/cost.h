#ifndef COST_H
#define COST_H

#include "image/image.h"
#include "shared_data.h"

#define PRECISION 4
#define SAMPLE_POS_RADIUS 1
#define SAMPLE_WEIGHT_RADIUS .02
#define MIN_RADIUS .1
#define MAX_RADIUS .1

struct gradient
{
    int dx[N_CELLS];
    int dy[N_CELLS];
    int dw[N_CELLS];
};

double compute_cost(struct image *image, struct voronoi_data *shared_data);
void compute_gradient(struct image *image, struct voronoi_data *shared_data,
                      struct gradient *out);

#endif /* ! COST_H */
