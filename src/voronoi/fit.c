#include "fit.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "cost.h"
#include "utils/utils.h"

enum error_code image_fit(struct image *image, struct voronoi_data *shared_data)
{
    double pos_learning_rate = LEARNING_RATE * MAX2(image->w, image->h);
    double weight_learning_rate = LEARNING_RATE;

    double prev_cost = -1;
    double cost;
    struct gradient gradient;
    bool done = false;
    int iteration = 0;
    while (true)
    {
        compute_gradient(image, shared_data, &gradient);

        double l = 0;
        for (int i = 0; i < N_CELLS; i++)
            l +=
                SQR(gradient.dx[i]) + SQR(gradient.dy[i]) + SQR(gradient.dw[i]);
        if (l)
        {
            double mul = 1 / sqrt(l / (N_CELLS * 3));
            for (int i = 0; i < N_CELLS; i++)
            {
                gradient.dx[i] *= mul;
                gradient.dy[i] *= mul;
                gradient.dw[i] *= mul;
            }
        }

        for (int i = 0; i < N_CELLS; i++)
        {
            struct cell *cell = shared_data->cells + i;
            cell->x -= gradient.dx[i] * pos_learning_rate;
            cell->y -= gradient.dy[i] * pos_learning_rate;
            cell->weight -= gradient.dw[i] * weight_learning_rate;
            cell->weight = 1; // TODO implement weights learning
        }
        pos_learning_rate *= LEARNING_RATE_DECAY;
        weight_learning_rate *= LEARNING_RATE_DECAY;

        cost = compute_cost(image, shared_data);

        if (ABS(prev_cost - cost) < COST_STAGNATE_THRESHOLD
            || cost > TARGET_FIT_PROPORTION)
        {
            done = true;
            break;
        }
        if (++iteration >= MAX_ITERATIONS)
            break;

        if (iteration % 1 == 0 && VERBOSE) // TODO restore modulo
            printf("Iteration %d/%d, accuracy score is %.3f%%\n", iteration,
                   MAX_ITERATIONS, (1 - cost) * 100);
    }

    if (VERBOSE)
    {
        if (done)
            printf(
                "Gradient descent done in %d iterations, final accuracy score "
                "is %d%%.\n",
                iteration, (int)round((1 - cost) * 100));
        else
            puts("Warning: gradient descent timed out.");
    }

    return SUCCESS;
}
