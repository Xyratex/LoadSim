/**
 simulator is a tool to the simulate situation when
 huge number of clients on big cluster.
 */

static int simul_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{

}

int simul_open(struct inode *inode, struct file *file)
{
	return !try_module_get(THIS_MODULE);
}

int lve_release(struct inode *inode, struct file *file)
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

	return 0;
out_dev:
	misc_deregister(&simul_dev);
	return ret;
}


static void __exit
simul_mod_cleanup(void)
{
	misc_deregister(&simul_dev);
}

module_init(simul_mod_init);
module_exit(simul_mod_cleanup);
