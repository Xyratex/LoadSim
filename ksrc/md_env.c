#include <linux/list.h>
#include <linux/slab.h>

#include "md_env.h"
#include "kapi.h"
#include "kdebug.h"
#include "vm_defs.h"
#include "vm_api.h"
#include "fifo.h"

static LIST_HEAD(md_clients);

int md_client_create(struct md_env **env, struct simul_ioctl_cli *data)
{
	struct md_env *ret;
	int rc;

	ret = kmalloc(sizeof *ret, GFP_KERNEL);
	if (ret == NULL) {
		err_print("can't alloc memory for md_env\n");
		return -ENOMEM;
	}
	memset(ret, 0, sizeof *ret);

	rc = vm_interpret_init(&ret->mde_vm, VM_DEF_STACK,
				data->sic_program, data->sic_programsz,
				ret);
	if (rc < 0)
		goto err;

	ret->mde_cli = lustre_cli;
	rc = ret->mde_cli.cli_init(&ret->mde_cli, NULL, NULL);
	if (rc < 0)
		goto err;

	list_add(&ret->mde_link, &md_clients);

	*env = ret;
	return 0;
err:
	md_client_destroy(ret);
	return rc;
}

void md_client_destroy(struct md_env *env)
{
	if (env->mde_vm != NULL)
		vm_interpret_fini(env->mde_vm);

	if (env->mde_cli.cli_fini)
		env->mde_cli.cli_fini(&env->mde_cli);

	kfree(env);
}

void md_clients_destroy(void)
{
	struct md_env *pos;
	struct md_env *tmp;

	list_for_each_entry_safe(pos, tmp, &md_clients, mde_link) {
		list_del(&pos->mde_link);
		md_client_destroy(pos);
	}
}

/*********** **********/
/**
	long VM_CALL_CD(char *)
	lookup name and update current directory pointer in
	env.
*/
static int md_call_cd(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *dirname;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	ret = md->mde_cli.cd(&md->mde_cli, dirname);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_MKDIR(char *)
*/
static int md_call_mkdir(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *dirname;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	ret = md->mde_cli.mkdir(&md->mde_cli, dirname);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_READIR(char *)
	get all directory pages from a md server
*/
static int md_call_readdir(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *dirname;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&dirname) < 0)
		return -ENODATA;

	ret = md->mde_cli.readdir(&md->mde_cli, dirname);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_UNLINK(char *);
*/
static int md_call_unlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = md->mde_cli.unlink(&md->mde_cli, name);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	long VM_CALL_OPEN(char *name, flag, ret);
*/
static int md_call_open(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	long flags;
	long reg;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, &flags) < 0)
		return -ENODATA;
	if (fifo_pop(f, &reg) < 0)
		return -ENODATA;

	ret = md->mde_cli.open(&md->mde_cli, name, flags, reg);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_CLOSE
*/
static int md_call_close(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	long reg;
	struct md_env *md = env;

	if (fifo_pop(f, &reg) < 0)
		return -ENODATA;
		
	ret = md->mde_cli.close(&md->mde_cli, reg);
	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}


/**
	VM_CALL_STAT
*/
static int md_call_stat(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = md->mde_cli.stat(&md->mde_cli, name);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_SETATTR
*/
static int md_call_setattr(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	long flags;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, &flags) < 0)
		return -ENODATA;

	ret = md->mde_cli.setattr(&md->mde_cli, name, flags);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_SOFTLINK
*/
static int md_call_softlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;

	ret = md->mde_cli.softlink(&md->mde_cli, name, new);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_HARDLINK
*/
static int md_call_hardlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;

	ret = md->mde_cli.hardlink(&md->mde_cli, name, new);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_READLINK
*/
static int md_call_readlink(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;

	ret = md->mde_cli.readlink(&md->mde_cli, name);

	return fifo_push(f, ret ? VM_RET_FAIL : VM_RET_OK);
}

/**
	VM_CALL_RENAME
*/
static int md_call_rename(void *env, struct fifo *f, uint32_t *ip)
{
	long ret;
	char *name;
	char *new;
	struct md_env *md = env;

	if (fifo_pop(f, (long *)&name) < 0)
		return -ENODATA;
	if (fifo_pop(f, (long *)&new) < 0)
		return -ENODATA;

	ret = md->mde_cli.rename(&md->mde_cli, name, new);

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
    {.hr_id = VM_MD_CALL_SETATTR, .hr_func = md_call_setattr },
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
