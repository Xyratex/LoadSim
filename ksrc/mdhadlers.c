#include "vm_defs.h"
#inclide "vm_api.h"

/**
	VM_CALL_CD
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
static int md_call_readdir(void *env, struct fifo *f, uint32_t *ip)
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


struct md_handler md_hld[] = {
    {.mh_id = VM_MD_CALL_CD, .mh_func = md_call_cd },
    {.mh_id = VM_MD_CALL_MKDIR, .mh_func = md_call_mkdir },
    {.mh_id = VM_MD_CALL_READIR, .mh_func = md_call_readdir },
    {.mh_id = VM_MD_CALL_UNLINK, .mh_func = md_call_unlink },
    {.mh_id = VM_MD_CALL_OPEN, .mh_func = md_call_open },
    {.mh_id = VM_MD_CALL_CLOSE, .mh_func = md_call_close },
    {.mh_id = VM_MD_CALL_STAT, .mh_func = md_call_stat },
    {.mh_id = VM_MD_CALL_SETATTR, .mh_func = md_call_setattr },
    {.mh_id = VM_MD_CALL_SOFTLINK, .mh_func = md_call_softlink },
    {.mh_id = VM_MD_CALL_HARDLINK, .mh_func = md_call_hardlink },
    {.mh_id = VM_MD_CALL_READLINK, .mh_func = md_call_readlink },
};

int md_handlers_init()
{
	return vm_handler_register(ARRAY_SIZE(md_hld), md_hld);
}

void md_handlers_unregister()
{
	vm_handler_unregister(ARRAY_SIZE(md_hld), md_hld);
}