#include "cost.h"

#include <pthread.h>
#include <string.h>

#include "utils/thread_conf.h"
#include "utils/utils.h"

struct thread_args
{
    const struct image *image;
    double cost1;

    // cell index bounds (a included, b excluded) for which to compute the
    // gradient
    int a;
    int b;

    // copy of the cells to avoid compute_cost interferences
    // greedy in memory, but that should not really be an issue
    struct cell cell_copies[N_CELLS];

    // shared variable to edit, should not need a mutex
    struct gradient *out;
};

double compute_cost(const struct image *image, const struct cell cells[N_CELLS])
{
    // TODO only check around the edited cell if applicable
    double total_cost = 0;

    for (int y = 0; y < image->h; y += PRECISION)
        for (int x = 0; x < image->w; x += PRECISION)
        {
            enum color_class color1 = get_pixel(image, x, y);

            double closest_dist = -1;
            double color2 = .5;
            for (int i = 0; i < N_CELLS; i++)
            {
                const struct cell *cell = cells + i;
                double dist = SQR(cell->x - x) + SQR(cell->y - y);
#ifdef WEIGHTED
                dist *= SQR(cell->weight);
#endif /* WEIGHTED */
                if (dist < closest_dist || closest_dist < 0)
                {
                    color2 = cell->training_color;
                    closest_dist = dist;
                }
            }

            total_cost += ABS(color1 - color2);
        }

    return total_cost * SQR(PRECISION) / image->size;
}

double compute_secondary_cost(const struct image *image,
                              const struct cell cells[N_CELLS], const int i)
{
    const struct cell *cell = cells + i;
    double x = cell->x, y = cell->y;

    double repulsion = -1;
    const double epsilon = 1e-9;

    // compute the closest distance to the other cells
    for (int j = 0; j < N_CELLS; j++)
    {
        if (i == j)
            continue;

        const struct cell *cell = cells + j;
        double dist = SQR(cell->x - x) + SQR(cell->y - y);
#ifdef WEIGHTED
        dist *= SQR(cell->weight);
#endif /* WEIGHTED */
        repulsion += 1 / (dist + epsilon);
    }

    // also take the borders into account
    repulsion += 1 / (SQR(2 * x) + epsilon);
    repulsion += 1 / (SQR(2 * y) + epsilon);
    repulsion += 1 / (SQR(2 * (image->w - x)) + epsilon);
    repulsion += 1 / (SQR(2 * (image->h - y)) + epsilon);

    return repulsion;
}

void *compute_gradient_part(void *data)
{
    struct thread_args *args = data;
    for (int i = args->a; i < args->b; i++)
    {
        struct cell *cell = args->cell_copies + i;
        const double x = cell->x, y = cell->y;
        const double c = cell->training_color;
#ifdef WEIGHTED
        const double w = cell->weight;
#endif /* WEIGHTED */

        // find points in bounds to avoid using e.g. x2-x conditions
        const double x2 = x < args->image->w / 2. ? x + SAMPLE_POS_RADIUS
                                                  : x - SAMPLE_POS_RADIUS,
                     y2 = y < args->image->h / 2. ? y + SAMPLE_POS_RADIUS
                                                  : y - SAMPLE_POS_RADIUS;
        const double c2 =
            c < .5 ? c + SAMPLE_COLOR_RADIUS : c - SAMPLE_COLOR_RADIUS;
#ifdef WEIGHTED
        const double w2 = w < (MIN_WEIGHT + MAX_WEIGHT) / 2.
            ? w + SAMPLE_WEIGHT_RADIUS
            : w - SAMPLE_WEIGHT_RADIUS;
#endif /* WEIGHTED */

        // for position changes, use a secondary cost to even out the cells
        double secondary1 =
            compute_secondary_cost(args->image, args->cell_copies, i);

        cell->x = x2;
        double cost2 = compute_cost(args->image, args->cell_copies);
        cost2 += (compute_secondary_cost(args->image, args->cell_copies, i)
                  - secondary1)
            * SECONDARY_COST_FACTOR;
        cell->x = x;
        args->out->dx[i] = (cost2 - args->cost1) / (x2 - x);

        cell->y = y2;
        cost2 = compute_cost(args->image, args->cell_copies);
        cost2 += (compute_secondary_cost(args->image, args->cell_copies, i)
                  - secondary1)
            * SECONDARY_COST_FACTOR;
        cell->y = y;
        args->out->dy[i] = (cost2 - args->cost1) / (y2 - y);

#ifdef WEIGHTED
        cell->weight = w2;
        cost2 = compute_cost(args->image, args->cell_copies);
        cell->weight = w;
        args->out->dw[i] = (cost2 - args->cost1) / (w2 - w);
#endif /* WEIGHTED */

        cell->training_color = c2;
        cost2 = compute_cost(args->image, args->cell_copies);
        cell->training_color = c;
        args->out->dc[i] = (cost2 - args->cost1) / (c2 - c);
    }

    return NULL;
}

enum error_code compute_gradient(const struct image *image,
                                 struct cell cells[N_CELLS],
                                 struct gradient *out, double cost1)
{
    // avoid allocations using stack-allocated static arrays
    static pthread_t threads[MAX_NUM_THREADS];
    static struct thread_args thread_args[MAX_NUM_THREADS];

    int a = 0;
    for (int i = 0; i < num_threads; i++)
    {
        int b = (i + 1) * N_CELLS / num_threads;

        struct thread_args *args = thread_args + i;
        args->image = image;
        args->cost1 = cost1;
        args->a = a;
        args->b = b;
        memcpy(args->cell_copies, cells, N_CELLS * sizeof(struct cell));
        args->out = out;

        if (pthread_create(threads + i, NULL, compute_gradient_part, args))
            return THREAD_ERROR;

        a = b;
    }

    bool err = false;
    for (int i = 0; i < num_threads; i++)
        err |= pthread_join(threads[i], NULL) != 0;

    return err ? THREAD_ERROR : SUCCESS;
}
