#include "vm_defs.h"
#include "vm_api.h"
#include "md_env.h"

/**
	VM_CALL_CD
	lookup name and update current directroy pointer in
	env.
*/
static int md_call_cd(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_MKDIR
*/
static int md_call_mkdir(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_READIR
	get all directory pages from a md server
*/
static int md_call_readdir(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_UNLINK
*/
static int md_call_unlink(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_OPEN
*/
static int md_call_open(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_CLOSE
*/
static int md_call_close(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}


/**
	VM_CALL_STAT
*/
static int md_call_stat(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_SETATTR
*/
static int md_call_setattr(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_SOFTLINK
*/
static int md_call_softlink(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_HARDLINK
*/
static int md_call_hardlink(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
}

/**
	VM_CALL_READLINK
*/
static int md_call_readlink(void *env, struct fifo *f, uint32_t *ip)
{
	return 0;
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
