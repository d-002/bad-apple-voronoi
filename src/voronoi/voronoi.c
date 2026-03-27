#include "voronoi.h"

#include <stdlib.h>

#include "apply.h"
#include "cost.h"
#include "fit.h"
#include "utils/errors.h"

// TODO remove
static int frame_count = 0;

enum error_code voronoi_process_frame(const char *source_path,
                                      const char *destination_path,
                                      struct voronoi_data *shared_data)
{
    struct image image;
    enum error_code err = image_load(source_path, &image);
    if (err != SUCCESS)
        return err;

    if (!shared_data->is_init)
        init_data(shared_data, &image);

    if (frame_count % 2 == 0)
        err = image_fit(&image, shared_data);
    if (frame_count % 2 == 0)
        if (err == SUCCESS)
            err = image_apply_voronoi(&image, shared_data);
    if (err == SUCCESS)
        err = image_save(&image, destination_path);
    frame_count++;

    free(image.pixels);
    return err;
}
