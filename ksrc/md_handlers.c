#include <linux/errno.h>

#include "vm_defs.h"
#include "vm_api.h"
#include "md_env.h"
#include "fifo.h"

/**
	long VM_CALL_CD(char *)
	lookup name and update current directory pointer in
	env.
*/
static int md_call_cd(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *dirname;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	long VM_CALL_MKDIR(char *)
*/
static int md_call_mkdir(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *dirname;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	long VM_CALL_READIR(char *)
	get all directory pages from a md server
*/
static int md_call_readdir(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *dirname;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	long VM_CALL_UNLINK(char *);
*/
static int md_call_unlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *name;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	long VM_CALL_OPEN(char *name, flag, ret);
*/
static int md_call_open(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *name;
	long flags;
	long reg;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, &flags) < 0)
		return -ENODATA;
	if (fifo_pop(f, &reg) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	VM_CALL_CLOSE
*/
static int md_call_close(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	long reg;

	if (fifo_pop(f, &reg) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}


/**
	VM_CALL_STAT
*/
static int md_call_stat(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *name;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	VM_CALL_SETATTR
*/
static int md_call_setattr(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *name;
	long flags;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, &flags) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	VM_CALL_SOFTLINK
*/
static int md_call_softlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *name;
	char *new;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;


	return fifo_push(f, ret);
}

/**
	VM_CALL_HARDLINK
*/
static int md_call_hardlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *name;
	char *new;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}

/**
	VM_CALL_READLINK
*/
static int md_call_readlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret = VM_RET_OK;
	char *name;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	return fifo_push(f, ret);
}


struct handler_reg md_hld[] = {
    {.hr_id = VM_MD_CALL_CD, .hr_func = md_call_cd },
    {.hr_id = VM_MD_CALL_MKDIR, .hr_func = md_call_mkdir },
    {.hr_id = VM_MD_CALL_READIR, .hr_func = md_call_readdir },
    {.hr_id = VM_MD_CALL_UNLINK, .hr_func = md_call_unlink },
    {.hr_id = VM_MD_CALL_OPEN, .hr_func = md_call_open },
    {.hr_id = VM_MD_CALL_CLOSE, .hr_func = md_call_close },
    {.hr_id = VM_MD_CALL_STAT, .hr_func = md_call_stat },
    {.hr_id = VM_MD_CALL_SETATTR, .hr_func = md_call_setattr },
    {.hr_id = VM_MD_CALL_SOFTLINK, .hr_func = md_call_softlink },
    {.hr_id = VM_MD_CALL_HARDLINK, .hr_func = md_call_hardlink },
    {.hr_id = VM_MD_CALL_READLINK, .hr_func = md_call_readlink },
};

int md_handlers_register()
{
	return vm_handler_register(ARRAY_SIZE(md_hld), md_hld);
}

void md_handlers_unregister()
{
	vm_handler_unregister(ARRAY_SIZE(md_hld), md_hld);
}
