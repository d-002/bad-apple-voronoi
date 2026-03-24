#include "files.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"

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

static char *join_path(char *path1, char *path2)
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

enum error_code process_file(char *name, char *source, char *destination,
                             struct voronoi_data **shared_data)
{
    char *source_path = join_path(source, name);
    char *destination_path = join_path(destination, name);
    if (source_path == NULL || destination_path == NULL)
    {
        free(source_path);
        free(destination_path);
        return ALLOC_ERROR;
    }

    enum error_code err =
        voronoi_process_frame(source_path, destination_path, shared_data);
    free(source_path);
    free(destination_path);

    return err;
}
