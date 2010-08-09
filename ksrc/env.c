#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "env.h"
#include "kapi.h"
#include "kdebug.h"
#include "vm_defs.h"
#include "vm_api.h"
#include "fifo.h"

#include "clients/client.h"

static LIST_HEAD(clients);

static void env_destroy(struct simul_env *env)
{
	if (env == NULL)
		return;

	if (env->se_vm != NULL)
		vm_interpret_fini(env->se_vm);

	if (env->u.se_md && env->u.se_md->cli_fini)
		env->u.se_md->cli_fini(env->se_data);

	if (env->se_name)
		kfree(env->se_name);

	kfree(env);
}

int env_create(struct simul_env **env, struct simul_ioctl_cli *data)
{
	struct simul_env *ret;
	int rc;

	ret = kmalloc(sizeof *ret, GFP_KERNEL);
	if (ret == NULL) {
		err_print("can't alloc memory for env\n");
		return -ENOMEM;
	}
	memset(ret, 0, sizeof *ret);

	ret->se_name = kstrdup(data->sic_name, GFP_KERNEL);
	if (ret->se_name == NULL) {
		err_print("can't alloca memory for env name\n");
		rc = -ENOMEM;
		goto err;
	}

	ret->se_id = data->sic_id;

	ret->u.se_md = &generic_cli;
	rc = ret->u.se_md->cli_init(&ret->se_data, data->sic_dst_fs,
				    data->sic_dst_nid);
	if (rc < 0)
		goto err;
		
	rc = vm_interpret_init(&ret->se_vm, VM_DEF_STACK,
				data->sic_program, data->sic_programsz,
				ret);
	if (rc < 0)
		goto err;

	list_add(&ret->se_link, &clients);

	*env = ret;
	return 0;
err:
	env_destroy(ret);
	return rc;
}

int env_run(struct simul_env *env)
{
	int rc;

	rc = env->u.se_md->cli_prerun(env->se_data);
	if (rc < 0) {
		err_print("prerun error %d\n", rc);
		return rc;
	}

	vm_interpret_run(env->se_vm);

	return 0;
}

void env_destroy_all(void)
{
	struct simul_env *pos;
	struct simul_env *tmp;

	list_for_each_entry_safe(pos, tmp, &clients, se_link) {
		list_del(&pos->se_link);
		env_destroy(pos);
	}
}

int env_count()
{
	int i = 0;
	struct list_head *tmp;

	list_for_each(tmp, &clients)
		i++;

	return i;
}

static void env_result_user(struct simul_env *env, struct simul_res *data)
{
	data->sr_cli = env->se_id;
	data->sr_res = env->se_vm->sv_rc;
	data->sr_ip = env->se_vm->sv_ip;
}

void env_results_get(struct simul_res *res)
{
	struct simul_env *pos;
	int i = 0;

	list_for_each_entry(pos, &clients, se_link) {
		env_result_user(pos, &res[i]);
		i++;
	}
}
