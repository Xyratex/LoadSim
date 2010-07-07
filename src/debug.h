#ifndef __MD_SIM_DEBUG__
#define __MD_SIM_DEBUG__

#define DPRINT(a, ...) if (1) { printf(a, ##__VA_ARGS__); }

#define err_print(a, ...) fprintf(stderr, a, ##__VA_ARGS__)

#endif