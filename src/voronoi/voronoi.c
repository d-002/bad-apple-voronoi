#include "voronoi.h"

#include <stdlib.h>

#include "apply.h"
#include "cost.h"
#include "fit.h"
#include "image/image.h"
#include "logger/logger.h"
#include "utils/errors.h"

// TODO remove
static int frame_count = 0;

enum error_code voronoi_process_frame(const char *source_path,
                                      const char *destination_path,
                                      struct voronoi_data **shared_data)
{
    struct image image;
    enum error_code err = image_load(source_path, &image);
    if (err != SUCCESS)
        return err;

    if (*shared_data == NULL)
    {
        *shared_data = calloc(1, sizeof(struct voronoi_data));
        for (int i = 0; i < N_CELLS; i++)
        {
            struct cell *cell = &(*shared_data)->cells[i];
            cell->x = (unsigned)rand() % image.w;
            cell->y = (unsigned)rand() % image.h;
#ifdef WEIGHTED
            cell->weight = (MIN_WEIGHT + MAX_WEIGHT) / 2.;
#endif /* WEIGHTED */
            cell->color = BLACK; // for now
            cell->training_color = .5; // TODO (double)rand() / RAND_MAX;
        }

        if (*shared_data == NULL)
        {
            log_alloc_error();
            free(image.pixels);
            return ALLOC_ERROR;
        }
    }

    // if (frame_count++ % 4 == 0)
    err = image_fit(&image, *shared_data);
    if (frame_count % 2)
        if (err == SUCCESS)
            err = image_apply_voronoi(&image, *shared_data);
    if (err == SUCCESS)
        err = image_save(&image, destination_path);

    free(image.pixels);
    return err;
}
