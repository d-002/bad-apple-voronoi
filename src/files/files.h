#ifndef FILES_H
#define FILES_H

#include <stdbool.h>
#include <stddef.h>

#include "utils/errors.h"
#include "voronoi/shared_data.h"

#define MAX_NUM_FILES 10000

enum error_code add_files_sorted(const char *path, char *names[MAX_NUM_FILES],
                                 size_t *len);
bool something_to_do(char *const names[MAX_NUM_FILES], const char *source,
                     const char *destination, size_t len,
                     long *latest_source_file);
enum error_code process_file(const char *name, const char *source,
                             const char *destination,
                             struct voronoi_data *shared_data);

#endif /* ! FILES_H */
