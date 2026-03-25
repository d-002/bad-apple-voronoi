#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stddef.h>

#include "cell.h"

struct voronoi_data
{
    struct cell cells[N_CELLS];
};

#endif /* ! SHARED_DATA_H */
