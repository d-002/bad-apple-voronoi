#ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 199309L
#endif /* ! _POSIX_C_SOURCE */
#include <linux/limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "files/files.h"
#include "logger/logger.h"
#include "signals/signals.h"
#include "utils/errors.h"
#include "utils/utils.h"

double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

enum error_code check_args(int argc, char *argv[], char **source,
                           char **destination)
{
    if (argc != 3)
    {
        logerror("Invalid arguments.");
        return ARGS_ERROR;
    }

    *source = argv[1];
    *destination = argv[2];

    struct stat s1, s2;
    stat(*source, &s1);
    stat(*destination, &s2);
    if (!S_ISDIR(s1.st_mode) || !S_ISDIR(s2.st_mode))
    {
        logerror("Source or destination are not valid directories.");
        return ARGS_ERROR;
    }

    return SUCCESS;
}

void progress_bar(struct voronoi_data *shared_data, int len)
{
    const double spent = now() - shared_data->start_time;
    const double eta =
        MAX2(spent * ((double)len / (shared_data->frame_index + 1) - 1), 0);
    const int hr_s = spent / 3600, min_s = (int)(spent / 60) % 60,
              sec_s = (int)(spent) % 60;
    int hr_e = eta / 3600, min_e = (int)(eta / 60) % 60,
        sec_e = (int)(eta) % 60;

    static const int size = 50;
    double prop;
    if (len == 0)
    {
        min_e = 0;
        sec_e = 0;
        prop = 1;
    }
    else
        prop = (double)shared_data->frame_index / len;
    const int count = round(prop * size);

    printf("Progress: [");
    for (int i = 0; i < count; i++)
        putchar('=');
    for (int i = count; i < size; i++)
        putchar(' ');
    printf("] %3d%%, spent: %d:%02d:%02d, eta: %d:%02d:%02d", (int)(prop * 100),
           hr_s, min_s, sec_s, hr_e, min_e, sec_e);
    putchar(VERBOSE ? '\n' : '\r');
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    char *source, *destination;
    enum error_code err = check_args(argc, argv, &source, &destination);
    if (err != SUCCESS)
        return err;

    char *names[MAX_NUM_FILES];
    size_t len;
    err = add_files_sorted(source, names, &len);
    if (err != SUCCESS)
        return err;

    long latest_source_file;
    if (something_to_do(names, source, destination, len, &latest_source_file))
    {
        printf("Converting all %ld files found.\n", len);

        struct voronoi_data *shared_data;
        err = load_data(latest_source_file, now(), SHARED_DATA_PATH,
                        &shared_data);
        if (err == SUCCESS)
            err = setup_signals();
        if (err == SUCCESS)
        {
            for (size_t i = 0; i < len && running; i++)
            {
                err = process_file(names[i], source, destination, shared_data);
                progress_bar(shared_data, len);
            }

            if (!running)
            {
                loginfo("Saving state to file");
                save_data(shared_data, SHARED_DATA_PATH);
            }

            putchar('\n');
            free(shared_data);
        }
    }
    else
        printf("%s: nothing to do.\n", argv[0]);

    for (size_t i = 0; i < len; i++)
        free(names[i]);

    return err;
}
