#ifndef CELL_H
#define CELL_H

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
};

#endif /* ! CELL_H */
