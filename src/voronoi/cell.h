#ifndef CELL_H
#define CELL_H

#define N_CELLS 20

enum class
{
    BLACK,
    WHITE,
};

struct cell
{
    int x;
    int y;
    double weight;

    enum class class;
};

#endif /* ! CELL_H */
