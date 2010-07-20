#include <linux/list.h>
#include <linux/slab.h>

#include "md_env.h"
#include "kapi.h"
#include "vm_defs.h"
#include "vm_api.h"

static LIST_HEAD(md_clients);

int md_client_create(struct md_env **env, struct simul_ioctl_cli *data)
{
	struct md_env *ret;
	int rc;

	ret = kmalloc(sizeof *ret, GFP_KERNEL);
	if (ret == NULL)
		return -ENOMEM;

	rc = vm_interpret_init(&ret->mde_vm, VM_DEF_STACK,
				data->sic_program, data->sic_programsz,
				ret);
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
	if (env->mde_vm)
		vm_interpret_fini(env->mde_vm);

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
