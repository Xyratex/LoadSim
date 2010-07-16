#include "fifo.h"
#include "vm_defs.h"

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
    {.mh_id = VM_SYS_USER, .mh_func = md_call_user },
    {.mh_id = VM_SYS_GROUP, .mh_func = md_call_group },
};

int sys_handlers_init()
{
	return vm_handler_register(ARRAY_SIZE(md_hld), md_hld);
}

void md_handlers_unregister()
{
	vm_handler_unregister(ARRAY_SIZE(md_hld), md_hld);
}