#ifndef THREAD_CONF_H
#define THREAD_CONF_H

#include <unistd.h>

#define MAX_NUM_THREADS 20

extern int num_threads;

void threads_init();

#endif /* ! THREAD_CONF_H */
