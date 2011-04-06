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
	char *dst;
	char *prefix;
	int len;
	int i;

	if (stack_pop(f, (long *)&dst) < 0)
		return -ENODATA;

	if (stack_pop(f, (long *)&prefix) < 0)
		return -ENODATA;

	len = strlen(prefix);
	memcpy(dst, prefix, len + 1);
	for (i = 0; i < len; i++) {
		if (dst[i] == 'X') {
			unsigned char byte;

			get_random_bytes(&byte, 1);
			dst[i] = '0' + (byte % 10);
		}
	}

	return stack_push(f, (long)dst);
}

#define pbuf_enough(l)	((ptr + (l) - print_buff) < VM_STRING_SZ)
static int sys_call_printf(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	char *format;
	char *print_buff;
	char *ptr;
	int len;
	int rc = 0;

	if (stack_pop(f, (long *)&print_buff) < 0)
		return -ENODATA;

	if (stack_pop(f, (long *)&format) < 0)
		return -ENODATA;

	print_buff[VM_STRING_SZ] = '\0';
	ptr = print_buff;
	DPRINT("format %s\n", format);
	while(*format != '\0') {
		if (*format != '%') {
			*ptr = *format;
			ptr ++;
			goto next_ch;
		}
		
		format++;
		switch(*format) {
		case 's': {
			char *data;
			if (stack_pop(f, (long *)&data) < 0) {
				rc = -ENODATA;
				break;
			}

			len = snprintf(ptr, 0, "%s", data);
			if (!pbuf_enough(len)) {
				rc = -EOVERFLOW;
				break;
			}
			sprintf(ptr, "%s", data);
			ptr += len;
			break;
		}
		case 'd': {
			long data;
			if (stack_pop(f, &data) < 0) {
				rc = -ENODATA;
				break;
			}
			len = snprintf(ptr, 0, "%ld", data);
			if (!pbuf_enough(len)) {
				rc = -EOVERFLOW;
				break;
			}
			DPRINT("long %ld\n", data);
			sprintf(ptr, "%ld", data);
			ptr += len;
			break;
		}
		case 'u': {
			long data;
			if (stack_pop(f, &data) < 0) {
				rc = -ENODATA;
				break;
			}
			len = snprintf(ptr, 0, "%lu", data);
			if (!pbuf_enough(len)) {
				rc = -EOVERFLOW;
				break;
			}
			sprintf(ptr, "%lu", data);
			ptr += len;
			break;
		}
		default:
			break;
		}
next_ch:
		format++;
	}
	*ptr = '\0';

	if (rc != 0)
		return rc;
	else
		return stack_push(f, (long)print_buff);
}

struct handler_reg sys_hld[] = {
    {.hr_id = VM_SYS_USER, .hr_func = sys_call_user },
    {.hr_id = VM_SYS_GROUP, .hr_func = sys_call_group },
    {.hr_id = VM_SYS_SLEEP, .hr_func = sys_call_sleep },
    {.hr_id = VM_SYS_RACE, .hr_func = sys_call_race },
    {.hr_id = VM_SYS_TMPNAME, .hr_func = sys_call_tmpname },
    {.hr_id = VM_SYS_PRINTF, .hr_func = sys_call_printf },
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
