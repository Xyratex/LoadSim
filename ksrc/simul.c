#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#include "kapi.h"
#include "kdebug.h"
#include "vm_defs.h" /* register / unregister */
#include "vm_api.h"
#include "md_env.h"

struct inode;

struct completion start;

static int client_thread(void *data)
{
	struct md_env *env;
	int rc;

	rc = md_client_create(&env, data);
	printk("cli create %d - %p\n", rc, env);
	if (rc < 0)
		return rc;

	complete(&start);
	/* wait until run event */
	printk("thread run\n");
	vm_interpret_run(env->mde_vm);

	return 0;
}


static int mdclient_create(struct simul_ioctl_cli *data)
{
	int rc = 0;
	struct task_struct *p;

	init_completion(&start);
	p = kthread_create(client_thread, data, data->sic_name);
	if (IS_ERR(p)) {
		rc = PTR_ERR(p);
		err_print("can't start thread %s - rc %d\n",
			  data->sic_name, rc);
		goto err;
	}
	wake_up_process(p);
	wait_for_completion(&start);
err:
	return rc;
}

static int simul_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int rc;

	switch (cmd) {
	case SIM_IOW_MDCLIENT:
		rc = mdclient_create((struct simul_ioctl_cli *)arg);
		break;
	case SIM_IOW_RUN:
	case SIM_IOW_RESULTS:
	case SIM_IOW_STATS:
	default:
		rc = -EINVAL;
		break;
	};
	return rc;
}

int simul_open(struct inode *inode, struct file *file)
{
	return !try_module_get(THIS_MODULE);
}

int simul_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static struct file_operations fops = {
	.owner	= THIS_MODULE,
	.open	= simul_open,
	.release = simul_release,
	.ioctl	= simul_ioctl,
};

static struct miscdevice simul_dev = {
	MISC_DYNAMIC_MINOR,
	SIMUL_DEV_NAME,
	&fops
};

static int __init
simul_mod_init(void)
{
	int ret;

	ret = misc_register(&simul_dev);
	printk(KERN_INFO "simul driver register status %d\n", ret);
	if (ret != 0)
		return ret;

	sys_handlers_register();
	md_handlers_register();

	return 0;
}


static void __exit
simul_mod_cleanup(void)
{
	misc_deregister(&simul_dev);

	sys_handlers_unregister();
	md_handlers_unregister();
	
	md_clients_destroy();
}
MODULE_LICENSE("GPL v2");
module_init(simul_mod_init);
module_exit(simul_mod_cleanup);
