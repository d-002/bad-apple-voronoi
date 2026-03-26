#ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 199309L
#endif /* ! _POSIX_C_SOURCE */
#include "now.h"

#include <time.h>

double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
