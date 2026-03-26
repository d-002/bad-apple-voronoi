#include "fit.h"

#include <math.h>
#include <stdbool.h>

#include "cost.h"
#include "logger/logger.h"
#include "signals/signals.h"
#include "utils/utils.h"

enum error_code image_fit(const struct image *image,
                          struct voronoi_data *shared_data)
{
    double pos_learning_rate = POS_LEARNING_RATE;
    double weight_learning_rate = WEIGHT_LEARNING_RATE;
    double color_learning_rate = COLOR_LEARNING_RATE;

    double prev_cost = -1;
    double cost = compute_cost(image, shared_data->cells);
    int consecutive_stagnate = 0;

    // try to skip the loop entirely
    bool done = 1 - cost > TARGET_FIT_PROPORTION;
    int iteration;

    struct gradient gradient;
    struct gradient moving_average = { 0 };
    for (iteration = 0; iteration < MAX_ITERATIONS && !done && running;
         iteration++)
    {
        compute_gradient(image, shared_data->cells, &gradient, cost);

        for (int i = 0; i < N_CELLS; i++)
        {
            double pos_l = SQR(gradient.dx[i]) + SQR(gradient.dy[i]);
            if (pos_l > 0)
            {
                const double mul = 1 / sqrt(pos_l);
                gradient.dx[i] *= mul;
                gradient.dy[i] *= mul;
            }
        }

        // Adam optimizer
        for (int i = 0; i < N_CELLS; i++)
        {
            moving_average.dx[i] = MOMENTUM * moving_average.dx[i]
                + (1 - MOMENTUM) * gradient.dx[i];
            moving_average.dy[i] = MOMENTUM * moving_average.dy[i]
                + (1 - MOMENTUM) * gradient.dy[i];
            moving_average.dc[i] = MOMENTUM * moving_average.dc[i]
                + (1 - MOMENTUM) * (gradient.dc[i] < 0 ? -1 : 1);
#ifdef WEIGHTED
            moving_average.dw[i] = MOMENTUM * moving_average.dw[i]
                + (1 - MOMENTUM) * gradient.dw[i];
#endif /* WEIGHTED */
        }

#ifdef WEIGHTED
        double min_weight = MAX_WEIGHT;
        double max_weight = MIN_WEIGHT;
#endif /* WEIGHTED */
        for (int i = 0; i < N_CELLS; i++)
        {
            struct cell *cell = shared_data->cells + i;
            cell->x -= moving_average.dx[i] * pos_learning_rate;
            cell->y -= moving_average.dy[i] * pos_learning_rate;
            cell->training_color -= moving_average.dc[i] * color_learning_rate;

            // make sure the cells stay in the bounds
            cell->x = MIN2(MAX2(0, cell->x), image->w - 1);
            cell->y = MIN2(MAX2(0, cell->y), image->h - 1);
            cell->training_color = MIN2(MAX2(0, cell->training_color), 1);
            cell->color = cell->training_color < .5 ? BLACK : WHITE;

#ifdef WEIGHTED
            cell->weight -= moving_average.dw[i] * weight_learning_rate;
            min_weight = MIN2(min_weight, cell->weight);
            max_weight = MAX2(max_weight, cell->weight);
#endif /* WEIGHTED */
        }

#ifdef WEIGHTED
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
#endif /* WEIGHTED */

        pos_learning_rate *= LEARNING_RATE_DECAY;
        weight_learning_rate *= LEARNING_RATE_DECAY;
        color_learning_rate *= LEARNING_RATE_DECAY;

        cost = compute_cost(image, shared_data->cells);

        if (ABS(prev_cost - cost) <= COST_STAGNATE_THRESHOLD)
        {
            if (++consecutive_stagnate >= 5)
                done = true;
        }
        else
            consecutive_stagnate = 0;
        if (1 - cost > TARGET_FIT_PROPORTION)
            done = true;

        prev_cost = cost;

        if ((iteration + 1) % 1 == 0 && VERBOSE) // TODO restore modulo
            loginfo("Iteration %d/%d, accuracy score is %.3f%%", iteration + 1,
                    MAX_ITERATIONS, (1 - cost) * 100);
    }

    if (VERBOSE && running)
    {
        if (done)
            loginfo("Gradient descent done in %d/%d iterations, final accuracy "
                    "score is %.3f%%.",
                    iteration + 1, MAX_ITERATIONS, (1 - cost) * 100);
        else
            logwarn("Gradient descent timed out.");
    }

    return SUCCESS;
}
