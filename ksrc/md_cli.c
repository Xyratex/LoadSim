#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>

#include "env.h"
#include "kdebug.h"
#include "vm_defs.h"
#include "vm_api.h"
#include "fifo.h"
#include "stats.h"
#include "clients/client.h"

#define MD_CALL(env, cmd, fnname, arg...)	\
		({						\
			struct timespec start = CURRENT_TIME;	\
			struct timespec diff;			\
			int rc;					\
			if (env->u.se_md == NULL)		\
				rc = -ENOSYS;			\
			else					\
				rc = env->u.se_md->fnname(env->se_data, ##arg); \
			diff = timespec_sub(CURRENT_TIME, start);	\
			stat_time_update(&env->se_stat_t[cmd - VM_MD_CALL_CD], &diff); \
			rc;					\
		})

int md_cli_init(struct simul_env *env, const char *fsname, const char *fsnid)
{
	int i;
	
	env->se_stat_tn = VM_MD_CALL_MAX - VM_MD_CALL_CD;
	env->se_stat_t = kmalloc(sizeof(*env->se_stat_t) * env->se_stat_tn,
				 GFP_KERNEL);
	if (env->se_stat_t == NULL)
		return -ENOMEM;

	for(i=0;i<env->se_stat_tn; i++)
		stat_time_init(&env->se_stat_t[i]);

	env->u.se_md = &generic_cli;
	return env->u.se_md->cli_init(&env->se_data, fsname, fsnid);

}

void md_cli_fini(struct simul_env *env)
{
	if (env->se_stat_t != NULL)
		kfree(env->se_stat_t);
	if (env->u.se_md && env->u.se_md->cli_fini)
		env->u.se_md->cli_fini(env->se_data);
}

int md_cli_prerun(struct simul_env *env)
{
	return env->u.se_md->cli_prerun(env->se_data);

}
/*********** **********/
/**
	long VM_CALL_CD(char *)
	lookup name and update current directory pointer in
	env.
*/
static int md_call_cd(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *dirname;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_CD, cd, dirname);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_MKDIR(char *)
*/
static int md_call_mkdir(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *dirname;
	long mode;

	if (fifo_pop(f, &mode) < 0)
		return -ENODATA;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_MKDIR, mkdir, dirname, mode);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_READIR(char *)
	get all directory pages from a md server
*/
static int md_call_readdir(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *dirname;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_READIR, readdir, dirname);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_UNLINK(char *);
*/
static int md_call_unlink(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_UNLINK, unlink, name);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_OPEN(char *name, flag, ret);
*/
static int md_call_open(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	long flags;
	long mode;
	long reg;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, &flags) < 0)
		return -ENODATA;
	if (fifo_pop(f, &mode) < 0)
		return -ENODATA;
	if (fifo_pop(f, &reg) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_OPEN, open, name, flags, mode, reg);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_CLOSE
*/
static int md_call_close(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	long reg;

	if (fifo_pop(f, &reg) < 0)
		return -ENODATA;
		
	ret = MD_CALL(env, VM_MD_CALL_CLOSE, close, reg);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_STAT
*/
static int md_call_stat(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_STAT, stat, name);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_CHMOD
*/
static int md_call_chmod(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	long mode;

	if (fifo_pop(f, &mode) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_CHMOD, chmod, name, mode);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_CHOWN
*/
static int md_call_chown(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	long uid;
	long gid;

	if (fifo_pop(f, &gid) < 0)
		return -ENODATA;
	if (fifo_pop(f, &uid) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_CHOWN, chown, name, uid, gid);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_CHTIME
*/
static int md_call_chtime(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	long time;

	if (fifo_pop(f, &time) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_CHTIME, chtime, name, time);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_TRUNCATE
*/
static int md_call_truncate(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	long size;

	if (fifo_pop(f, &size) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_TRUNCATE, truncate, name, size);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_SOFTLINK
*/
static int md_call_softlink(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;

	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_SOFTLINK, softlink, name, new);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_HARDLINK
*/
static int md_call_hardlink(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;

	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_HARDLINK, hardlink, name, new);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_READLINK
*/
static int md_call_readlink(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_READLINK, readlink, name);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_RENAME
*/
static int md_call_rename(struct simul_env *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;

	ret = MD_CALL(env, VM_MD_CALL_RENAME, rename, name, new);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}



struct handler_reg md_hld[] = {
    {.hr_id = VM_MD_CALL_CD, .hr_func = md_call_cd },
    {.hr_id = VM_MD_CALL_MKDIR, .hr_func = md_call_mkdir },
    {.hr_id = VM_MD_CALL_READIR, .hr_func = md_call_readdir },
    {.hr_id = VM_MD_CALL_UNLINK, .hr_func = md_call_unlink },
    {.hr_id = VM_MD_CALL_OPEN, .hr_func = md_call_open },
    {.hr_id = VM_MD_CALL_CLOSE, .hr_func = md_call_close },
    {.hr_id = VM_MD_CALL_STAT, .hr_func = md_call_stat },
    {.hr_id = VM_MD_CALL_CHMOD, .hr_func = md_call_chmod },
    {.hr_id = VM_MD_CALL_CHOWN, .hr_func = md_call_chown },
    {.hr_id = VM_MD_CALL_CHTIME, .hr_func = md_call_chtime },
    {.hr_id = VM_MD_CALL_TRUNCATE, .hr_func = md_call_truncate },
    {.hr_id = VM_MD_CALL_SOFTLINK, .hr_func = md_call_softlink },
    {.hr_id = VM_MD_CALL_HARDLINK, .hr_func = md_call_hardlink },
    {.hr_id = VM_MD_CALL_READLINK, .hr_func = md_call_readlink },
    {.hr_id = VM_MD_CALL_RENAME, .hr_func = md_call_rename },
};

int md_handlers_register()
{
	return vm_handler_register(ARRAY_SIZE(md_hld), md_hld);
}

void md_handlers_unregister()
{
	vm_handler_unregister(ARRAY_SIZE(md_hld), md_hld);
}
