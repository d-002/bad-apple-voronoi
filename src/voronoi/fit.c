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
    int iteration = 0;

    struct gradient gradient;
    struct gradient moving_average = { 0 };
    for (; iteration < MAX_ITERATIONS && !done && running; iteration++)
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
        }

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
        }

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

#ifdef VERBOSE
        if ((iteration + 1) % 1 == 0) // TODO restore modulo
            loginfo("Iteration %d/%d, accuracy score is %.3f%%", iteration + 1,
                    MAX_ITERATIONS, (1 - cost) * 100);
#endif /* VERBOSE */
    }

#ifdef VERBOSE
    if (running && done)
    {
        if (iteration == 0)
            loginfo(
                "Gradient descent skipped due to satisfactory initial cost.");
        else
            loginfo("Gradient descent done in %d/%d iterations, final accuracy "
                    "score is %.3f%%.",
                    iteration, MAX_ITERATIONS, (1 - cost) * 100);
    }
#endif /* VERBOSE */
    if (running && !done)
        logwarn("Gradient descent timed out.");

    return SUCCESS;
}
