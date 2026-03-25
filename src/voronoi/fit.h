#ifndef FIT_H
#define FIT_H

#include "image/image.h"
#include "shared_data.h"
#include "utils/errors.h"

// base learning rates, different for pos and weights because of the scale
#define POS_LEARNING_RATE 1
#define WEIGHT_LEARNING_RATE .01
#define COLOR_LEARNING_RATE .5
// learning rate decay each iteration
#define LEARNING_RATE_DECAY .995
// proportion of pixels that should be correct to end fitting process
#define TARGET_FIT_PROPORTION .8
// threshold to stop early if the target cost was not reached
#define COST_STAGNATE_THRESHOLD 1e-9
#define MAX_ITERATIONS 100

enum error_code image_fit(const struct image *image,
                          struct voronoi_data *shared_data);

#endif /* ! FIT_H */
