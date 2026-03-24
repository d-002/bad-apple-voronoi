#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "logger/logger.h"
#include "utils/errors.h"
#include "voronoi/voronoi.h"

#define MAX_NUM_FILES 10000

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

char *join_path(char *path1, char *path2)
{
    size_t length1 = strlen(path1);
    size_t length2 = strlen(path2);
    char *joined = calloc(length1 + length2 + 2, sizeof(char));
    if (joined == NULL)
    {
        log_alloc_error();
        return NULL;
    }

    memcpy(joined, path1, length1);
    joined[length1] = '/';
    memcpy(joined + length1 + 1, path2, length2);

    return joined;
}

enum error_code add_files_sorted(char *path, char *names[MAX_NUM_FILES],
                                 size_t *len)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        logerror("Could not open source dir.");
        return ARGS_ERROR;
    }

    struct dirent *ent;
    *len = 0;

    while ((ent = readdir(dir)) != NULL)
    {
        char *name = ent->d_name;
        if (memcmp(name, "frame", 5 * sizeof(char)))
            continue;

        if (*len >= MAX_NUM_FILES)
        {
            logerror("Not enough space to store and store all file names");
            return ALLOC_ERROR;
        }

        size_t length = strlen(name);
        char *copy = calloc(length + 1, sizeof(char));
        if (copy == NULL)
        {
            log_alloc_error();
            return ALLOC_ERROR;
        }

        memcpy(copy, name, length);

        for (size_t i = 0; i <= *len; i++)
            if (i == *len || strcmp(names[i], name) > 0)
            {
                for (size_t j = *len; j > i; j--)
                    names[j] = names[j - 1];
                names[i] = copy;
                break;
            }

        (*len)++;
    }

    closedir(dir);
    return SUCCESS;
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
        char *source_path = join_path(source, names[i]);
        char *destination_path = join_path(destination, names[i]);
        if (source_path == NULL || destination_path == NULL)
        {
            free(source_path);
            free(destination_path);
            err = ALLOC_ERROR;
            break;
        }
        err =
            voronoi_process_frame(source_path, destination_path, &shared_data);
        free(source_path);
        free(destination_path);
        if (err != SUCCESS)
            break;

        progress_bar(i + 1, len);
    }
    putchar('\n');

    for (size_t i = 0; i < len; i++)
        free(names[i]);
    free(shared_data);

    return err;
}
