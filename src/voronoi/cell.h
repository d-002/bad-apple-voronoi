#ifndef CELL_H
#define CELL_H

// number of cells in a given frame
#define N_CELLS 20

enum color_class
{
    BLACK = 0,
    WHITE = 1,
};

struct cell
{
    double x;
    double y;
    double weight;

    enum color_class color;

    // floating-point interpolation used during training, later casted into its
    // final color
    double training_color;
};

#endif /* ! CELL_H */
