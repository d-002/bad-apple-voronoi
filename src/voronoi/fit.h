#ifndef FIT_H
#define FIT_H

#include "image/image.h"
#include "shared_data.h"
#include "utils/errors.h"

// base learning rate, normalized later for cells positions because of the scale
#define LEARNING_RATE .1
// learning rate decay each iteration
#define LEARNING_RATE_DECAY .99
// proportion of pixels that should be correct to end fitting process
#define TARGET_FIT_PROPORTION .9
// threshold to stop early if the target cost was not reached
#define COST_STAGNATE_THRESHOLD .001
#define MAX_ITERATIONS 1000
#define VERBOSE true

enum error_code image_fit(struct image *image,
                          struct voronoi_data *shared_data);

#endif /* ! FIT_H */
