#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include "env.h"
#include "kapi.h"
#include "kdebug.h"
#include "vm_defs.h"
#include "vm_api.h"
#include "md_cli.h"
#include "stats.h"

static LIST_HEAD(clients);

static void env_destroy(struct simul_env *env)
{
	if (env == NULL)
		return;

	if (env->se_vm != NULL)
		vm_interpret_fini(env->se_vm);

	md_cli_fini(env);

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

	rc = md_cli_init(ret, data->sic_dst_fs, data->sic_dst_nid);
	if (rc < 0)
		goto err;
		
	rc = vm_interpret_init(&ret->se_vm, VM_DEF_STACK,
				data->sic_program, data->sic_programsz,
				data->sic_regs,
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

	rc = md_cli_prerun(env);
	if (rc < 0) {
		err_print("can't call prerun\n");
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

/* to md_cli ? */
static int env_stat_to_user(uint32_t id, struct op_time_stat *orig,
			    struct simul_stat_op __user *new)
{
	struct simul_stat_op tmp;

	tmp.sso_op_id = id + VM_MD_CALL_CD;
	tmp.sso_min_time = timespec_to_ns(&orig->ot_min) / NSEC_PER_USEC;
	tmp.sso_max_time = timespec_to_ns(&orig->ot_max) / NSEC_PER_USEC;
	tmp.sso_avg_time = timespec_to_ns(&orig->ot_sum) / NSEC_PER_USEC;
	if (orig->ot_count)
		tmp.sso_avg_time /= orig->ot_count;

	if (copy_to_user(new, &tmp, sizeof(*new)))
		return -EFAULT;

	return 0;
}


static int env_result_to_user(struct simul_env *env, uint32_t __user *res,
			      uint32_t __user *ip, uint64_t __user *total_time,
			      struct simul_stat_op __user *data)
{
	int i;
	uint64_t time;

	if (copy_to_user(ip, &env->se_vm->sv_ip, sizeof(*ip)))
		return -EFAULT;

	if (copy_to_user(res, &env->se_vm->sv_rc, sizeof(*res)))
		return -EFAULT;

        time = timespec_to_ns(&env->se_vm->sv_time) / NSEC_PER_USEC;
	if (copy_to_user(total_time, &time, sizeof(*total_time)))
		return -EFAULT;

	for (i = 0; i < env->se_stat_tn; i++)
		if (env_stat_to_user(i, &env->se_stat_t[i], (data + i)))
			return -EFAULT;

	return 0;
}

int env_results_get(uint32_t id, uint32_t __user *res, uint32_t __user *ip,
                    uint64_t __user *time, 
		    struct simul_stat_op __user *data)
{
	struct simul_env *pos;

	list_for_each_entry(pos, &clients, se_link) {
		if (pos->se_id == id) {
			return env_result_to_user(pos, res, ip, time, data);
		}
	}

	return -ENOENT;
}

