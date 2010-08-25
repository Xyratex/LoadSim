#ifndef __MD_SIM_DEBUG__
#define __MD_SIM_DEBUG__

#include "loadsim_c.h"

#define DPRINT(a, ...) if (DEBUG) { printf(a, ##__VA_ARGS__); }

#define err_print(a, ...) fprintf(stderr, a, ##__VA_ARGS__)

#endif
