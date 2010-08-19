#ifndef __SIMUL_STATS__
#define __SIMUL_STATS__

#include <linux/types.h>
#include <linux/time.h>

struct op_stat {
	uint32_t	st_min;
	uint32_t	st_max;
	uint32_t	st_count;
	uint64_t	st_sum;
};

void stat_update_val(struct op_stat *st, uint32_t value);

struct op_time_stat {
	struct timespec	ot_min;
	struct timespec	ot_max;
	uint32_t	ot_count;
	struct timespec	ot_sum;
};

void stat_time_init(struct op_time_stat *st);
void stat_time_update(struct op_time_stat *st, struct timespec *val);
#endif
