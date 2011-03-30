#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/bitops.h>
#include <linux/random.h>

#include "compat.h"
#include "kdebug.h"
#include "stack.h"
#include "vm_defs.h"
#include "vm_api.h"

/**
 VM_SYS_USER
*/
static int sys_call_user(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long uid;
	struct cred *cred;

	if (stack_pop(f, &uid))
		return -ENODATA;

	if ((cred = prepare_creds())) {
		cred->uid = uid;
		cred->fsuid = uid;
		commit_creds(cred);
	}

	return 0;
}

/**
 VM_SYS_GROUP
*/
static int sys_call_group(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long gid;
	struct cred *cred;

	if (stack_pop(f, &gid))
		return -ENODATA;

	if ((cred = prepare_creds())) {
		cred->gid = gid;
		cred->fsgid = gid;
		commit_creds(cred);
	}


	return 0;
}

static int sys_call_sleep(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long time;

	if (stack_pop(f, &time))
		return -ENODATA;

	msleep(time);

	return 0;
}

struct _vm_race {
	long			flags;
	wait_queue_head_t	wait;
};

struct _vm_race vm_race[VM_MAX_RACES];

static int sys_call_race(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long raceid;

	if (stack_pop(f, &raceid))
		return -ENODATA;

	if ((raceid < 0) || (raceid > VM_MAX_RACES))
		return -EINVAL;

	if (test_and_set_bit(0, &vm_race[raceid].flags)) {
		/** old bit is non zero - second hit */
		vm_race[raceid].flags = 0;
		wake_up(&vm_race[raceid].wait);
	} {
		wait_event(vm_race[raceid].wait, vm_race[raceid].flags == 0);
	}

	return 0;
}

static int sys_call_tmpname(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	char *prefix;
	int len;
	int i;

	if (stack_pop(f, (long *)&prefix) < 0)
		return -ENODATA;

	len = strlen(prefix);
	for (i = 0; i < len; i++) {
		if (prefix[i] == 'X') {
			unsigned char byte;

			get_random_bytes(&byte, 1);
			prefix[i] = '0' + (byte % 10);
		}
	}

	return stack_push(f, (long)prefix);
}

struct handler_reg sys_hld[] = {
    {.hr_id = VM_SYS_USER, .hr_func = sys_call_user },
    {.hr_id = VM_SYS_GROUP, .hr_func = sys_call_group },
    {.hr_id = VM_SYS_SLEEP, .hr_func = sys_call_sleep },
    {.hr_id = VM_SYS_RACE, .hr_func = sys_call_race },
    {.hr_id = VM_SYS_TMPNAME, .hr_func = sys_call_tmpname },
};

int sys_handlers_register()
{
	int i;

	for (i=0; i < VM_MAX_RACES; i++) {
		vm_race[i].flags = 0;
		init_waitqueue_head(&vm_race[i].wait);
	}

	return vm_handler_register(ARRAY_SIZE(sys_hld), sys_hld);
}

void sys_handlers_unregister()
{
	vm_handler_unregister(ARRAY_SIZE(sys_hld), sys_hld);
}
