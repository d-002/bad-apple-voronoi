#ifndef CELL_H
#define CELL_H

#include "utils/utils.h"

// number of cells in a given frame
#define N_CELLS 200

enum color_class
{
    BLACK = 0,
    WHITE = 1,
};

struct cell
{
    double x;
    double y;
#ifdef WEIGHTED
    double weight;
#endif /* WEIGHTED */

    enum color_class color;

    // floating-point interpolation used during training, later casted into its
    // final color
    double training_color;
};

#endif /* ! CELL_H */
