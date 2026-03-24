#ifndef VORONOI_H
#define VORONOI_H

#include <stddef.h>

#include "cell.h"
#include "utils/errors.h"

struct voronoi_data
{
    struct cell cells[N_CELLS];
    size_t n_cells;
};

enum error_code voronoi_process_frame(const char *source_path,
                                      const char *destination_path,
                                      struct voronoi_data **shared_data);

#endif /* ! VORONOI_H */
