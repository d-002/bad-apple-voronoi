#ifndef CELL_H
#define CELL_H

enum color_class
{
    BLACK,
    WHITE,
};

struct cell
{
    int x;
    int y;
    double weight;

    enum color_class color;
};

#endif /* ! CELL_H */
