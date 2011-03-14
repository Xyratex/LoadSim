#include <linux/time.h>
#include <linux/types.h>

#include "kdebug.h"
#include "stats.h"

void stat_update_val(struct op_stat *st, uint32_t val)
{
	if (st->st_min < val)
		st->st_min = val;
	if (st->st_max > val)
		st->st_max = val;
	st->st_count ++;
	st->st_sum += val;
}

void stat_time_init(struct op_time_stat *st)
{
	st->ot_min.tv_sec = 1000; /** more then timeout  */
	st->ot_min.tv_nsec = 0;
	st->ot_max.tv_sec = 0;
	st->ot_max.tv_nsec = 0;
	st->ot_count = 0;
	st->ot_sum.tv_sec = 0;
	st->ot_sum.tv_nsec = 0;
}

void stat_time_update(struct op_time_stat *st, struct timespec *val)
{
	DPRINT("update stat %d:%ld\n", (int)val->tv_sec, val->tv_nsec);

	if (timespec_compare(&st->ot_min, val) > 0) {
		st->ot_min = *val;
	}
	if (timespec_compare(&st->ot_max, val) < 0) {
		st->ot_max = *val;
	}
	st->ot_count ++;
	st->ot_sum.tv_sec += val->tv_sec;
	st->ot_sum.tv_nsec += val->tv_nsec;
}

#ifndef HAVE_NORM_TIMESPEC
void set_normalized_timespec(struct timespec *ts, time_t sec, long nsec)
{
	while (nsec >= NSEC_PER_SEC) {
		nsec -= NSEC_PER_SEC;
		++sec;
	}
	while (nsec < 0) {
		nsec += NSEC_PER_SEC;
		--sec;
	}
	ts->tv_sec = sec;
	ts->tv_nsec = nsec;

}
#endif