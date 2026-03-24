#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "files/files.h"
#include "logger/logger.h"
#include "utils/errors.h"

static int start;

int now()
{
    return 0;
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
    int spent = now() - start;
    int eta = spent * (len / (i + 1) - 1);
    int min_s = spent / 60, sec_s = spent % 60;
    int min_e = eta / 60, sec_e = eta % 60;

    static const int size = 50;
    float prop;
    if (len == 0)
    {
        min_e = 0;
        sec_e = 0;
        prop = 1;
    }
    else
        prop = (float)i / len;
    int count = prop * size;

    printf("Progress: [");
    for (int i = 0; i < count; i++)
        putchar('=');
    for (int i = count; i < size; i++)
        putchar(' ');
    printf("] %d%%, spent: %02d:%02d, eta: %02d:%02d\r", (int)(prop * 100),
           min_s, sec_s, min_e, sec_e);
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

    start = now();
    struct voronoi_data *shared_data = NULL;
    for (size_t i = 0; i < len; i++)
    {
        err = process_file(names[i], source, destination, &shared_data);
        progress_bar(i + 1, len);
    }
    putchar('\n');

    for (size_t i = 0; i < len; i++)
        free(names[i]);
    free(shared_data);

    return err;
}
