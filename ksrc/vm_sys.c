#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sched.h>

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


struct handler_reg sys_hld[] = {
    {.hr_id = VM_SYS_USER, .hr_func = sys_call_user },
    {.hr_id = VM_SYS_GROUP, .hr_func = sys_call_group },
};

int sys_handlers_register()
{
	return vm_handler_register(ARRAY_SIZE(sys_hld), sys_hld);
}

void sys_handlers_unregister()
{
	vm_handler_unregister(ARRAY_SIZE(sys_hld), sys_hld);
}
