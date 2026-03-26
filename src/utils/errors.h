#ifndef ERRORS_H
#define ERRORS_H

enum error_code
{
    SUCCESS = 0,
    ARGS_ERROR = 1,
    IO_ERROR = 2,
    SIGNAL_ERROR = 3,
    THREAD_ERROR = 4,

    ALLOC_ERROR = -1,
};

#endif /* ! ERRORS_H */
