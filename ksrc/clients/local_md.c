#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/sched.h>
#include <linux/statfs.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include "kdebug.h"
#include "kapi.h"
#include "compat.h"
#include "client.h"
#include "md_cli_priv.h"


static int local_cli_destroy(struct md_private *lp)
{
	return generic_cli_destroy(lp);
}

int mount_local(char *path, struct vfsmount **mnt, struct dentry **dent)
{
	int retval;
	struct nameidata nd;

	retval = path_lookup(path, LOOKUP_FOLLOW, &nd);
	if (retval)
		return retval;
	*mnt = mntget(sim_nd_mnt(nd));
	*dent = dget(sim_nd_dentry(nd));
	sim_path_put(&nd);

	return 0;
}

static int local_cli_create(struct md_private **cli, const void __user *private)
{
	struct md_private *ret = NULL;
	char *opt = NULL;
	const struct local_mnt_opt *mnt = private;
	int rc;

	opt = getname(mnt->mnt);
	if (opt == NULL)
		return -ENOMEM;

	rc = generic_cli_create(&ret);
	if (rc < 0)
		return rc;

	rc = mount_local(opt, &ret->lp_mnt, &ret->lp_root);
	if (rc < 0) {
		ret->lp_mnt = NULL;
		ret->lp_root = NULL;
		goto error;
	}

	*cli = ret;
	rc = 0;
	ret = NULL;
error:
	if (ret)
		generic_cli_destroy(ret);
	if (opt)
		putname(opt);
	return rc;
}

struct md_client md_local_cli = {
	.cli_init 	= local_cli_create,
	.cli_prerun	= generic_cli_prerun,
	.cli_fini	= local_cli_destroy,

	.cd		= generic_cli_cd,
	.mkdir		= generic_cli_mkdir,
	.readdir	= generic_cli_readdir,
	.unlink		= generic_cli_unlink,
	.open		= generic_cli_open,
	.close		= generic_cli_close,
	.stat		= generic_cli_stat,
	.chmod		= generic_cli_chmod,
	.chown		= generic_cli_chown,
	.chtime		= generic_cli_chtime,
	.truncate	= generic_cli_truncate,
	.lookup		= generic_cli_lookup,
	.softlink	= generic_cli_softlink,
	.hardlink	= generic_cli_hardlink,
	.readlink	= generic_cli_readlink,
	.rename		= generic_cli_rename,
};
