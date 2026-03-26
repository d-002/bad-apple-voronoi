#ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 200112L
#endif /* ! _POSIX_C_SOURCE */

#include "signals.h"

#include <signal.h>
#include <stdlib.h>

#include "logger/logger.h"

bool running;

void handler(int signum)
{
    switch (signum)
    {
    case SIGPIPE:
        return;
    case SIGINT:
        loginfo("Saving state to file and stopping...");
        loginfo("Please wait for the end of the current fitting iteration.");
        running = false;
        return;
    default:
        return;
    }
}

enum error_code setup_signals(void)
{
    running = true;

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handler;

    if (sigemptyset(&sa.sa_mask) < 0)
        return SIGNAL_ERROR;
    if (sigaction(SIGINT, &sa, NULL) < 0)
        return SIGNAL_ERROR;

    return SUCCESS;
}
