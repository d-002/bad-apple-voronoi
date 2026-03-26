#include "shared_data.h"

#include <stdlib.h>

#include "logger/logger.h"
#include "utils/utils.h"

enum error_code save_data(struct voronoi_data *data, char *cache_path)
{
    data++;
    cache_path++;
    return SUCCESS;
}

static struct voronoi_data *shared_data_create(long start_time)
{
    struct voronoi_data *shared_data = calloc(1, sizeof(struct voronoi_data));
    if (shared_data == NULL)
    {
        log_alloc_error();
        return NULL;
    }

    shared_data->is_init = false;
    shared_data->start_time = start_time;
    shared_data->frame_index = 0;

    return shared_data;
}

void init_data(struct voronoi_data *shared_data, struct image *image)
{
    for (int i = 0; i < N_CELLS; i++)
    {
        struct cell *cell = shared_data->cells + i;
        cell->x = (unsigned)rand() % image->w;
        cell->y = (unsigned)rand() % image->h;
#ifdef WEIGHTED
        cell->weight = (MIN_WEIGHT + MAX_WEIGHT) / 2.;
#endif /* WEIGHTED */
        cell->training_color = (double)rand() / RAND_MAX;
        cell->color = cell->training_color < .5 ? BLACK : WHITE;
    }

    shared_data->is_init = true;
}

enum error_code load_data(long latest_source_file, long start_time,
                          char *cache_path, struct voronoi_data **out)
{
    latest_source_file++; /////
    cache_path++; /////

    *out = shared_data_create(start_time);
    if (*out == NULL)
        return ALLOC_ERROR;

    return SUCCESS;
}
