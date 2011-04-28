#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>

#include "env.h"
#include "kapi.h"
#include "kdebug.h"
#include "vm_defs.h"
#include "vm_api.h"
#include "stack.h"
#include "stats.h"
#include "client.h"

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

struct md_client *mdtp [SM_MNT_MAX] = { 
    [SM_MNT_LOCAL] = &md_local_cli,
    [SM_MNT_LUSTRE] = &md_lustre_cli,
};

int md_cli_init(struct simul_env *env, const struct simul_mnt *mnt)
{
	int i;

	if (mnt->sm_type <= SM_MNT_NONE || mnt->sm_type >= SM_MNT_MAX)
		return -EINVAL;

	env->se_stat_tn = VM_MD_CALL_MAX - VM_MD_CALL_CD;
	env->se_stat_t = kmalloc(sizeof(*env->se_stat_t) * env->se_stat_tn,
				 GFP_KERNEL);
	if (env->se_stat_t == NULL)
		return -ENOMEM;

	for(i=0;i<env->se_stat_tn; i++)
		stat_time_init(&env->se_stat_t[i]);

	env->u.se_md = mdtp[mnt->sm_type];
	return env->u.se_md->cli_init(&env->se_data, &mnt->sm_u);
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
static int md_call_cd(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *dirname;

	if (stack_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	DPRINT("md_cd %s\n", dirname);

	ret = MD_CALL(env, VM_MD_CALL_CD, cd, dirname);
	return stack_push(f, ret);
}

/**
	long VM_CALL_MKDIR(char *)
*/
static int md_call_mkdir(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *dirname;
	long mode;

	if (stack_pop(f, &mode) < 0)
		return -ENODATA;

	if (stack_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	DPRINT("md_mkdir %s %lx\n", dirname, mode);
	ret = MD_CALL(env, VM_MD_CALL_MKDIR, mkdir, dirname, mode);
	return stack_push(f, ret);
}

/**
	long VM_CALL_READIR(char *)
	get all directory pages from a md server
*/
static int md_call_readdir(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *dirname;

	if (stack_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	DPRINT("md_readdir %s\n", dirname);
	ret = MD_CALL(env, VM_MD_CALL_READIR, readdir, dirname);
	return stack_push(f, ret);
}

/**
	long VM_CALL_UNLINK(char *);
*/
static int md_call_unlink(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;

	DPRINT("md_unlink %s\n", name);
	ret = MD_CALL(env, VM_MD_CALL_UNLINK, unlink, name);
	return stack_push(f, ret);
}

/**
	long VM_CALL_OPEN(char *name, flag, ret);
*/
static int md_call_open(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	long flags;
	long mode;
	long reg;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, &flags) < 0)
		return -ENODATA;
	if (stack_pop(f, &mode) < 0)
		return -ENODATA;
	if (stack_pop(f, &reg) < 0)
		return -ENODATA;

	DPRINT("md_open %s:%lx:%lx:%ld\n", name, flags, mode, reg);
	ret = MD_CALL(env, VM_MD_CALL_OPEN, open, name, flags, mode, reg);
	return stack_push(f, ret);
}

/**
	VM_CALL_CLOSE
*/
static int md_call_close(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	long reg;

	if (stack_pop(f, &reg) < 0)
		return -ENODATA;

	DPRINT("md_close %ld\n", reg);
	ret = MD_CALL(env, VM_MD_CALL_CLOSE, close, reg);
	return stack_push(f, ret);
}

/**
	VM_CALL_STAT
*/
static int md_call_stat(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;

	DPRINT("md_stat %s\n", name);
	ret = MD_CALL(env, VM_MD_CALL_STAT, stat, name);
	return stack_push(f, ret);
}

/**
	VM_CALL_CHMOD
*/
static int md_call_chmod(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	long mode;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, &mode) < 0)
		return -ENODATA;

	DPRINT("md_chmod %s:%lx\n", name, mode);
	ret = MD_CALL(env, VM_MD_CALL_CHMOD, chmod, name, mode);
	return stack_push(f, ret);
}

/**
	VM_CALL_CHOWN
*/
static int md_call_chown(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	long uid;
	long gid;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, &uid) < 0)
		return -ENODATA;
	if (stack_pop(f, &gid) < 0)
		return -ENODATA;

	DPRINT("md_chown %s:%lu:%lu\n", name, uid, gid);
	ret = MD_CALL(env, VM_MD_CALL_CHOWN, chown, name, uid, gid);
	return stack_push(f, ret);
}

/**
	VM_CALL_CHTIME
*/
static int md_call_chtime(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	long time;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, &time) < 0)
		return -ENODATA;

	DPRINT("md_chtime %s:%ld\n", name, time);
	ret = MD_CALL(env, VM_MD_CALL_CHTIME, chtime, name, time);
	return stack_push(f, ret);
}

/**
	VM_CALL_TRUNCATE
*/
static int md_call_truncate(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	long size;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, &size) < 0)
		return -ENODATA;

	DPRINT("md_truncate %s:%ld\n", name, size);
	ret = MD_CALL(env, VM_MD_CALL_TRUNCATE, truncate, name, size);
	return stack_push(f, ret);
}

/**
	VM_CALL_SOFTLINK
*/
static int md_call_softlink(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, (long *)&new) < 0)
		return -ENODATA;

	DPRINT("md_softlink %s <> %s\n", name, new);
	ret = MD_CALL(env, VM_MD_CALL_SOFTLINK, softlink, name, new);
	return stack_push(f, ret);
}

/**
	VM_CALL_HARDLINK
*/
static int md_call_hardlink(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, (long *)&new) < 0)
		return -ENODATA;

	DPRINT("md_hardlink %s <> %s\n", name, new);
	ret = MD_CALL(env, VM_MD_CALL_HARDLINK, hardlink, name, new);
	return stack_push(f, ret);
}

/**
	VM_CALL_READLINK
*/
static int md_call_readlink(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;

	DPRINT("md_readlink %s\n", name);
	ret = MD_CALL(env, VM_MD_CALL_READLINK, readlink, name);
	return stack_push(f, ret);
}

/**
	VM_CALL_RENAME
*/
static int md_call_rename(struct simul_env *env, struct stack *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;

	if (stack_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (stack_pop(f, (long *)&new) < 0)
		return -ENODATA;

	DPRINT("md_rename %s -> %s\n", name, new);
	ret = MD_CALL(env, VM_MD_CALL_RENAME, rename, name, new);
	return stack_push(f, ret);
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
