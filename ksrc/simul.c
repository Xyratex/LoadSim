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
#include <linux/mutex.h>    /* used mutex for traversing through linked list */
#include <linux/proc_fs.h>  /* contains  procfs structures and APIs */
#include <linux/seq_file.h> /* for seq file interface */

#include "kapi.h"
#include "kdebug.h"
#include "vm_defs.h" /* register / unregister */
#include "vm_api.h"
#include "env.h"

#define DIRNAME "ldsim"
#define FILENAME "report_interval"

static struct proc_dir_entry *stat;
static struct proc_dir_entry *ldsim;
extern struct list_head clients;
static DEFINE_MUTEX(env_mtx);
static unsigned total_opr;

struct inode;

struct completion start;

static atomic_t clients_cnt = ATOMIC_INIT(0);
struct timespec start_time;
struct timespec finish_time;

static atomic_t clients_run = ATOMIC_INIT(0);

DECLARE_WAIT_QUEUE_HEAD(poll_waiting);

enum sim_state {
	SIM_SETUP,
	SIM_RUN,
	SIM_TERM,
};

static enum sim_state state = SIM_SETUP;
static DECLARE_WAIT_QUEUE_HEAD(clients_run_waitq);


static void *ldsim_start(struct seq_file *seq, loff_t *pos)
{
    mutex_lock(&env_mtx);
    return seq_list_start(&clients, *pos);
}

static void *ldsim_next(struct seq_file *seq, void *v, loff_t *pos)
{
    return seq_list_next(v, &clients, pos);
}

static void ldsim_stop(struct seq_file *seq, void *v)
{
	struct timespec delta;
	struct timespec interval_time = CURRENT_TIME;
	delta = timespec_sub(interval_time, start_time);
	seq_printf(seq, "total of %u clients: %u\n", atomic_read(&clients_cnt), total_opr);
	seq_printf(seq, "time: %lldms\n", (timespec_to_ns(&delta) / NSEC_PER_MSEC));
    mutex_unlock(&env_mtx);
}

static int ldsim_show(struct seq_file *seq, void *v)
{
    struct simul_env *pos = list_entry(v, struct simul_env, se_link);

    seq_printf(seq, "%s    %u\n", pos->se_name, atomic_read(&pos->opr_counter));
	total_opr += atomic_read(&pos->opr_counter);
    atomic_set(&pos->opr_counter, 0);
    return 0;
}
static struct seq_operations ldsim_ops = {
    .start  = ldsim_start,
    .next   = ldsim_next,
    .stop   = ldsim_stop,
    .show   = ldsim_show,
};

static int ldsim_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &ldsim_ops);
}

static const struct file_operations proc_fileops = {
    .owner  = THIS_MODULE,
    .open   = ldsim_open,
    .read   = seq_read,
    .llseek = seq_lseek,
    .release= seq_release,
};

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
	wake_up(&poll_waiting);

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

void clients_destroy(void)
{
	env_destroy_all();
	atomic_set(&clients_cnt, 0);
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

	DPRINT("got cmd %x\n", cmd);
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
		clients_destroy();
		rc = 0;
		break;
	default:
		rc = -ENOSYS;
		break;
	};
	return rc;
}

ssize_t simul_read(struct file *f, char __user *buf, size_t len, loff_t *offset)
{
	int rc  = 0;
	int size = 0;
	
	
	return rc < 0 ? rc : size;
}

int simul_open(struct inode *inode, struct file *file)
{
	return !try_module_get(THIS_MODULE);
}

int simul_release(struct inode *inode, struct file *file)
{
	clients_destroy();
	module_put(THIS_MODULE);
	return 0;
}

static unsigned int simul_poll (struct file * file, poll_table * wait)
{
	unsigned int mask = 0;

	poll_wait(file, &poll_waiting, wait);
	if (atomic_read(&clients_run) == 0) {
		finish_time = CURRENT_TIME;
		mask |= POLLHUP;
	}
#if 0
	if (histroy_size() > 0) {
		mask |= POLLIN;
	}
#endif
	DPRINT("poll cli %d - %u\n", atomic_read(&clients_run), mask);
	return mask;
}


static struct file_operations fops = {
	.owner	= THIS_MODULE,
	.open	= simul_open,
	.read   = simul_read, 
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

	ldsim = proc_mkdir(DIRNAME, NULL);
    if (ldsim == NULL)
    {
        remove_proc_entry(DIRNAME, NULL);
        return -ENOMEM;
    }
	stat = proc_create(FILENAME, 0666, ldsim, &proc_fileops);
    if (stat == NULL)
    {
        remove_proc_entry(FILENAME, ldsim);
        remove_proc_entry(DIRNAME, NULL);
        return -ENOMEM;
    }
	return 0;
}


static void __exit
simul_mod_cleanup(void)
{
	misc_deregister(&simul_dev);

	sys_handlers_unregister();
	md_handlers_unregister();
	
	env_destroy_all();

	remove_proc_entry(FILENAME, ldsim);
	remove_proc_entry(DIRNAME, NULL);
}
MODULE_LICENSE("GPL v2");
module_init(simul_mod_init);
module_exit(simul_mod_cleanup);
