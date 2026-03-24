#include "voronoi.h"

#include <stdlib.h>

#include "image/image.h"
#include "logger/logger.h"
#include "utils/errors.h"

enum error_code voronoi_process_frame(const char *source_path,
                                      const char *destination_path,
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

    struct image image;
    enum error_code err = image_load(source_path, &image);
    if (err != SUCCESS)
        return err;

    err = image_save(&image, destination_path);

    free(image.pixels);
    return err;
}
