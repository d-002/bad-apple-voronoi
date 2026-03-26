#include "thread_conf.h"

#include "logger/logger.h"
#include "utils.h"

int num_threads;

void threads_init()
{
    num_threads = MIN2(sysconf(_SC_NPROCESSORS_ONLN), MAX_NUM_THREADS);
    loginfo("Will use %d threads for gradient computing.", num_threads);
}
