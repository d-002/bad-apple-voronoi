#ifndef COST_H
#define COST_H

#include "image/image.h"
#include "utils/errors.h"

// how much to downsample the image
#define PRECISION 8
// sample distance for gradient estimation
#define SAMPLE_POS_RADIUS PRECISION
#define SAMPLE_WEIGHT_RADIUS .05
#define SAMPLE_COLOR_RADIUS .01
// weight for the secondary cost, see relevant code
#define SECONDARY_COST_FACTOR .01
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
                    const struct cell cells[N_CELLS]);
enum error_code compute_gradient(const struct image *image,
                                 struct cell cells[N_CELLS],
                                 struct gradient *out, double cost1);

#endif /* ! COST_H */
