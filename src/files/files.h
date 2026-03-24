#ifndef FILES_H
#define FILES_H

#include <stddef.h>

#include "utils/errors.h"
#include "voronoi/voronoi.h"

#define MAX_NUM_FILES 10000

enum error_code add_files_sorted(char *path, char *names[MAX_NUM_FILES],
                                 size_t *len);
enum error_code process_file(char *name, char *source, char *destination,
                             struct voronoi_data **shared_data);

#endif /* ! FILES_H */
