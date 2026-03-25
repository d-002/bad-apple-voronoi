#ifndef VORONOI_H
#define VORONOI_H

#include "shared_data.h"
#include "utils/errors.h"

enum error_code voronoi_process_frame(const char *source_path,
                                      const char *destination_path,
                                      struct voronoi_data **shared_data);

#endif /* ! VORONOI_H */
