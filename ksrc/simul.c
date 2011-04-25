#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <asm/atomic.h>

#include "kapi.h"
#include "kdebug.h"
#include "vm_defs.h" /* register / unregister */
#include "vm_api.h"
#include "env.h"

struct inode;

struct completion start;

static atomic_t clients_cnt = ATOMIC_INIT(0);
struct timespec start_time;
struct timespec finish_time;

static atomic_t clients_run = ATOMIC_INIT(0);

static DECLARE_WAIT_QUEUE_HEAD(clients_wait);

enum sim_state {
	SIM_SETUP,
	SIM_RUN,
	SIM_TERM,
};

static enum sim_state state = SIM_SETUP;
static DECLARE_WAIT_QUEUE_HEAD(clients_run_waitq);

static int client_thread(void *d)
{
	struct simul_env *env = d;

	snprintf(current->comm, sizeof(current->comm), "%s", env->se_name);

	atomic_inc(&clients_run);
	atomic_inc(&clients_cnt);
	complete(&start);

	wait_event(clients_run_waitq, state != SIM_SETUP);
	if (state == SIM_TERM)
		return 0;

	/* wait until run event */
	DPRINT("thread run\n");
	env_run(env);

	atomic_dec(&clients_run);
	wake_up(&clients_wait);

	return 0;
}

static int client_create(struct simul_ioctl_cli __user *d)
{
	int rc = 0;
	pid_t p;
	struct simul_env *env;
	struct simul_ioctl_cli data;

	if (copy_from_user(&data, d, sizeof *d))
		return -EFAULT;

	rc = env_create(&env, &data);
	DPRINT("cli create %d - %p\n", rc, env);
	if (rc < 0)
		return rc;

	init_completion(&start);
	p = kernel_thread(client_thread, env, 0);
	if (p < 0) {
		rc = p;
		err_print("can't start thread %s - rc %d\n",
			  data.sic_name, rc);
		goto err;
	}
	wait_for_completion(&start);
err:
	return rc;
}

int user_results_get(struct simul_ioctl_user_res __user *data)
{
	struct simul_ioctl_user_res _data;

	if (copy_from_user(&_data, data, sizeof(_data)))
		return -EFAULT;

	return env_results_get(_data.ss_cli, _data.ss_res, _data.ss_ip, 
                               _data.ss_time, _data.ss_stats);
}

int system_results_get(struct simul_ioctl_system_res __user *data)
{
	struct simul_ioctl_system_res _data;
	struct timespec diff;

	_data.ssr_ncli = atomic_read(&clients_cnt);

	diff = timespec_sub(finish_time, start_time);
	_data.ssr_time = timespec_to_ns(&diff) / NSEC_PER_USEC;

	return copy_to_user(data, &_data, sizeof(*data));
}

static int simul_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int rc;

	DPRINT("got cmd %d\n", cmd);
	switch (cmd) {
	case SIM_IOW_MDCLIENT:
		rc = client_create((struct simul_ioctl_cli *)arg);
		break;
	case SIM_IOW_RUN:
		rc = 0;
		state = SIM_RUN;
		start_time = CURRENT_TIME;
		wake_up(&clients_run_waitq);
		break;
	case SIM_IOW_USER_RESULTS:
		rc = user_results_get((struct simul_ioctl_user_res *)arg);
		break;
	case SIM_IOW_SYSTEM_RESULTS:
		rc = system_results_get((struct simul_ioctl_system_res *)arg);
		break;
	case SIM_IOW_DESTROY_CLI:
		env_destroy_all();
		rc = 0;
		break;
	default:
		rc = -ENOSYS;
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
	env_destroy_all();

	module_put(THIS_MODULE);
	return 0;
}

static unsigned int simul_poll (struct file * file, poll_table * wait)
{
	unsigned int mask = 0;

	poll_wait(file, &clients_wait, wait);
	if (atomic_read(&clients_run) == 0) {
		finish_time = CURRENT_TIME;
		mask |= POLLIN | POLLRDNORM;
	}

	DPRINT("poll cli %d - %u\n", atomic_read(&clients_run), mask);
	return mask;
}


static struct file_operations fops = {
	.owner	= THIS_MODULE,
	.open	= simul_open,
	.release = simul_release,
	.ioctl	= simul_ioctl,
	.poll	= simul_poll,
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
	
	env_destroy_all();
}
MODULE_LICENSE("GPL v2");
module_init(simul_mod_init);
module_exit(simul_mod_cleanup);
