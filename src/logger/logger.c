#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

void log_inner(const char *prefix, const char *format, va_list args)
{
    fputs(prefix, stderr);

    fputs("bad_apple: ", stderr);

    vfprintf(stderr, format, args);
    putc('\n', stderr);
}

void loginfo(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_inner("[INFO]  ", format, args);
    va_end(args);
}

void logwarn(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_inner("[WARN]  ", format, args);
    va_end(args);
}

void logerror(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_inner("[ERROR] ", format, args);
    va_end(args);
}

void log_alloc_error()
{
    logerror("Failed to allocate memory.");
}
