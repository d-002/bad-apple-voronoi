#include "shared_data.h"

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger/logger.h"
#include "utils/now.h"
#include "utils/utils.h"
#include "voronoi/cost.h"

enum error_code read_obj(const int fd, void *out, const ssize_t size)
{
    return read(fd, out, size) == size ? SUCCESS : IO_ERROR;
}

enum error_code write_obj(const int fd, const void *data, const ssize_t size)
{
    return write(fd, data, size) == size ? SUCCESS : IO_ERROR;
}

enum error_code save_data(const struct voronoi_data *shared_data,
                          const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        logerror("Failed to open file for writing: '%s'", path);
        return IO_ERROR;
    }

    const int n_cells = N_CELLS;
    enum error_code err = write_obj(fd, &n_cells, sizeof(int));
    if (err != SUCCESS)
        goto write_err;
    double spent = now() - shared_data->start_time;
    err = write_obj(fd, &spent, sizeof(double));
    if (err != SUCCESS)
        goto write_err;
    err = write_obj(fd, &shared_data->frame_index, sizeof(int));
    if (err != SUCCESS)
        goto write_err;

    for (int i = 0; i < N_CELLS; i++)
    {
        err = write_obj(fd, shared_data->cells + i, sizeof(struct cell));
        if (err != SUCCESS)
            goto write_err;
    }

    close(fd);
    loginfo("Successfully saved state to file");
    return SUCCESS;

write_err:
    close(fd);
    logerror("Failed to read from file: '%s'", path);
    return err;
}

static struct voronoi_data *shared_data_create()
{
    struct voronoi_data *shared_data = calloc(1, sizeof(struct voronoi_data));
    if (shared_data == NULL)
    {
        log_alloc_error();
        return NULL;
    }

    shared_data->is_init = false;
    shared_data->start_time = now();
    shared_data->frame_index = 0;

    return shared_data;
}

void init_data(struct voronoi_data *shared_data, const struct image *image)
{
    for (int i = 0; i < N_CELLS; i++)
    {
        struct cell *cell = shared_data->cells + i;
        cell->x = (unsigned)rand() % image->w;
        cell->y = (unsigned)rand() % image->h;
        cell->training_color = (double)rand() / RAND_MAX;
        cell->color = cell->training_color < .5 ? BLACK : WHITE;
    }

    shared_data->is_init = true;
}

enum error_code restore_data_from_file(const char *path,
                                       struct voronoi_data *shared_data)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        logerror("Failed to open file for reading: '%s'", path);
        return IO_ERROR;
    }

    int n_cells;
    enum error_code err = read_obj(fd, &n_cells, sizeof(int));
    if (err != SUCCESS)
        goto read_err;
    if (n_cells != N_CELLS)
    {
        logwarn("Found invalid cache file (wrong number of cells, expected %d "
                "but got %d).",
                N_CELLS, n_cells);
        close(fd);
        return SUCCESS;
    }

    double spent;
    err = read_obj(fd, &spent, sizeof(double));
    if (err != SUCCESS)
        goto read_err;
    shared_data->start_time = now() - spent;
    err = read_obj(fd, &shared_data->frame_index, sizeof(int));
    if (err != SUCCESS)
        goto read_err;

    for (int i = 0; i < N_CELLS; i++)
    {
        err = read_obj(fd, shared_data->cells + i, sizeof(struct cell));
        if (err != SUCCESS)
            goto read_err;
    }

    shared_data->is_init = true;
    close(fd);

    int hr = spent / 3600, min = (int)round(spent / 60) % 60,
        sec = (int)round(spent) % 60;
    loginfo("Successfully restored state from cache file.");
    loginfo(
        "Current frame is now %d, with %d:%02d:%02d spent in previous runs.",
        shared_data->frame_index + 1, hr, min, sec);
    return SUCCESS;

read_err:
    close(fd);
    logerror("Failed to read from file: '%s'", path);
    return err;
}

enum error_code load_data(const long latest_source_file, const char *path,
                          struct voronoi_data **out)
{
    *out = shared_data_create();
    if (*out == NULL)
        return ALLOC_ERROR;

    enum error_code err = SUCCESS;

    // check if a cache file exists and is valid
    if (access(path, F_OK) == 0)
    {
        struct stat s;
        stat(path, &s);

        if (s.st_mtime >= latest_source_file)
            err = restore_data_from_file(path, *out);
        else
            logwarn("Ignored and deleted obsolete cache file.");

        if (remove(path))
            logwarn("Failed to remove cache file.");
    }

    return err;
}
