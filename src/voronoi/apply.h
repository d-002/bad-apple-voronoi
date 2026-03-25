#ifndef APPLY_H
#define APPLY_H

#include "image/image.h"
#include "shared_data.h"
#include "utils/errors.h"

enum error_code image_apply_voronoi(struct image *image,
                                    const struct voronoi_data *shared_data);

#endif /* ! APPLY_H */
