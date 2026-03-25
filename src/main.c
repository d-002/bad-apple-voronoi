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
#include "utils/errors.h"
#include "utils/utils.h"

static double start;

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

void progress_bar(int i, int len)
{
    const double spent = now() - start;
    const double eta = MAX2(spent * ((double)len / (i + 1) - 1), 0);
    const int min_s = spent / 60, sec_s = (int)(spent) % 60;
    int min_e = eta / 60, sec_e = (int)(eta) % 60;

    static const int size = 50;
    double prop;
    if (len == 0)
    {
        min_e = 0;
        sec_e = 0;
        prop = 1;
    }
    else
        prop = (double)i / len;
    const int count = round(prop * size);

    printf("Progress: [");
    for (int i = 0; i < count; i++)
        putchar('=');
    for (int i = count; i < size; i++)
        putchar(' ');
    printf("] %3d%%, spent: %02d:%02d, eta: %02d:%02d", (int)(prop * 100),
           min_s, sec_s, min_e, sec_e);
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

    if (something_to_do(names, source, destination, len))
    {
        printf("Converting all %ld files found.\n", len);

        // share some data across frames, since most of them introduce little
        // change keeping the old Voronoi cells can be advantageous
        struct voronoi_data *shared_data = NULL;

        start = now();
        for (size_t i = 0; i < len; i++)
        {
            err = process_file(names[i], source, destination, &shared_data);
            progress_bar(i + 1, len);
        }

        putchar('\n');
        free(shared_data);
    }
    else
        printf("%s: nothing to do.\n", argv[0]);

    for (size_t i = 0; i < len; i++)
        free(names[i]);

    return err;
}
