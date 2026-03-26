#ifndef SIGNALS_H
#define SIGNALS_H

#include <stdbool.h>

#include "utils/errors.h"

enum error_code setup_signals(void);

extern bool running;

#endif /* ! SIGNALS_H */
