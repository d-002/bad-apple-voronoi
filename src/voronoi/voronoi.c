#include "voronoi.h"

#include <stdlib.h>

#include "logger/logger.h"
#include "utils/errors.h"

enum error_code voronoi_process_frame(char *path,
                                      struct voronoi_data **shared_data)
{
    if (*shared_data == NULL)
    {
        *shared_data = calloc(1, sizeof(struct voronoi_data));
        if (*shared_data == NULL)
        {
            log_alloc_error();
            return ALLOC_ERROR;
        }
    }

    loginfo("Processing frame '%s'", path);

    return SUCCESS;
}
