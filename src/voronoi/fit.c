#include "fit.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "cost.h"
#include "utils/utils.h"

enum error_code image_fit(const struct image *image,
                          struct voronoi_data *shared_data)
{
    double pos_learning_rate = POS_LEARNING_RATE;
    double weight_learning_rate = WEIGHT_LEARNING_RATE;
    double color_learning_rate = COLOR_LEARNING_RATE;

    double prev_cost = -1;
    double cost;
    int consecutive_stagnate = 0;

    struct gradient gradient;
    bool done = false;
    int iteration;
    for (iteration = 0; iteration < MAX_ITERATIONS; iteration++)
    {
        compute_gradient(image, shared_data, &gradient);

        for (int i = 0; i < N_CELLS; i++)
        {
            double l = SQR(gradient.dx[i]) + SQR(gradient.dy[i])
                + SQR(gradient.dw[i]) + SQR(gradient.dc[i]);
            if (l)
            {
                const double mul = 1 / sqrt(l);
                gradient.dx[i] *= mul;
                gradient.dy[i] *= mul;
                gradient.dw[i] *= mul;
                gradient.dc[i] *= mul;
            }
        }

        double min_weight = MAX_WEIGHT;
        double max_weight = MIN_WEIGHT;
        for (int i = 0; i < N_CELLS; i++)
        {
            struct cell *cell = shared_data->cells + i;
            cell->x -= gradient.dx[i] * pos_learning_rate;
            cell->y -= gradient.dy[i] * pos_learning_rate;
            cell->weight -= gradient.dw[i] * weight_learning_rate;
            cell->weight = 1; // TODO restore weights
            cell->training_color -= gradient.dc[i] * color_learning_rate;

            // make sure the cells stay in the bounds
            cell->x = MIN2(MAX2(0, cell->x), image->w - 1);
            cell->y = MIN2(MAX2(0, cell->y), image->h - 1);
            min_weight = MIN2(min_weight, cell->weight);
            max_weight = MAX2(min_weight, cell->weight);
            cell->training_color = MIN2(MAX2(0, cell->training_color), 1);
            cell->color = cell->training_color < .5 ? BLACK : WHITE;
        }

        // normalize weights
        if (min_weight < max_weight)
        {
            double span = max_weight - min_weight;
            for (int i = 0; i < N_CELLS; i++)
            {
                struct cell *cell = shared_data->cells + i;
                cell->weight = (cell->weight - min_weight) / span
                        * (MAX_WEIGHT - MIN_WEIGHT)
                    + MIN_WEIGHT;
            }
        }

        pos_learning_rate *= LEARNING_RATE_DECAY;
        weight_learning_rate *= LEARNING_RATE_DECAY;
        color_learning_rate *= LEARNING_RATE_DECAY;

        cost = compute_cost(image, shared_data);

        if (ABS(prev_cost - cost) <= COST_STAGNATE_THRESHOLD)
        {
            if (++consecutive_stagnate >= 5)
            {
                done = true;
                break;
            }
        }
        else
            consecutive_stagnate = 0;
        if (1 - cost > TARGET_FIT_PROPORTION)
        {
            done = true;
            break;
        }

        prev_cost = cost;

        if ((iteration + 1) % 1 == 0 && VERBOSE) // TODO restore modulo
            printf("Iteration %d/%d, accuracy score is %.3f%%\n", iteration + 1,
                   MAX_ITERATIONS, (1 - cost) * 100);
    }

    if (VERBOSE)
    {
        if (done)
            printf("Gradient descent done in %d/%d iterations, final accuracy "
                   "score is %.3f%%.\n",
                   iteration + 1, MAX_ITERATIONS, (1 - cost) * 100);
        else
            puts("Warning: gradient descent timed out.");
    }

    return SUCCESS;
}
