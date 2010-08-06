#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/bitops.h>

#include "fifo.h"
#include "vm_defs.h"
#include "vm_api.h"

/**
 VM_SYS_USER
*/
static int sys_call_user(void *env, struct fifo *f, uint32_t *ip)
{
	long uid;

	if (fifo_pop(f, &uid))
		return -ENODATA;

	current->uid = current->fsuid = uid;

	return 0;
}

/**
 VM_SYS_GROUP
*/
static int sys_call_group(void *env, struct fifo *f, uint32_t *ip)
{
	long gid;

	if (fifo_pop(f, &gid))
		return -ENODATA;

	current->gid = current->fsgid = gid;

	return 0;
}


static int sys_call_sleep(void *env, struct fifo *f, uint32_t *ip)
{
	long time;

	if (fifo_pop(f, &time))
		return -ENODATA;

	msleep(time);

	return 0;
}

struct _vm_race {
	long				flags;
	struct wait_queue_head_t	wait;
};

struct _vm_race vm_race[VM_MAX_RACES];

static int sys_call_race(void *env, struct fifo *f, uint32_t *ip)
{
	long raceid;

	if (fifo_pop(f, &raceid))
		return -ENODATA;

	if ((raceid < 0) || (raceid > VM_MAX_RACES))
		return -EINVAL;

	if (test_and_set_bit(0, &vm_race[raceid].flags)) {
		/** old bit is non zero - second hit */
		vm_race[raceid].flags = 0;
		wake_up(&vm_race[raceid].wait);
	} {
		wait_event(&vm_race[raceid].wait, vm_race[raceid].flags == 0);
	}

	return 0;
}

struct handler_reg sys_hld[] = {
    {.hr_id = VM_SYS_USER, .hr_func = sys_call_user },
    {.hr_id = VM_SYS_GROUP, .hr_func = sys_call_group },
    {.hr_id = VM_SYS_SLEEP, .hr_func = sys_call_sleep },
    {.hr_id = VM_SYS_RACE, .hr_func = sys_call_race },
};

int sys_handlers_register()
{
	int i;

	for (i=0; i < VM_MAX_RACES; i++) {
		vm_race[i].hit = 0;
		vm_race[i].wait = __WAIT_QUEUE_HEAD_INITIALIZER(vm_race[i].wait);
	}

	return vm_handler_register(ARRAY_SIZE(sys_hld), sys_hld);
}

void sys_handlers_unregister()
{
	vm_handler_unregister(ARRAY_SIZE(sys_hld), sys_hld);
}
