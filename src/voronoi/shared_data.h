#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stddef.h>

#include "cell.h"
#include "image/image.h"
#include "utils/errors.h"

#define SHARED_DATA_PATH "saved_state.bin"

/* Share some data across frames: since most of them introduce little change,
 * keeping the Voronoi cells from the previous frame can lead to faster fitting
 * times.
 * This data can also be saved and loaded back from a persistent file to split
 * the process in multiple sessions.
 */
struct voronoi_data
{
    bool is_init;
    struct cell cells[N_CELLS];
    double start_time;
    int frame_index;
};

/*
 * Save the shared data to a file, regardless of whether it exists or not.
 * This file will be able to be used to continue fitting later from the current
 * state
 */
enum error_code save_data(const struct voronoi_data *shared_data,
                          const char *path);

void init_data(struct voronoi_data *shared_data, const struct image *image);

/*
 * Load back the stored shared data from a file.
 * If a source file is more recent than the cache file, if it is invalid (with
 * respect to constants like N_CELLS) or it doesn't exist, the data is created
 * but uninitialized.
 */
enum error_code load_data(const long latest_source_file, const char *path,
                          struct voronoi_data **out);

#endif /* ! SHARED_DATA_H */
