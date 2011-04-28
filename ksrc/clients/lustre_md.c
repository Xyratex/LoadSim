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

static const char lustre_fs[] = "lustre";

static struct vfsmount *mount_lustre(char *dev, char *opt)
{
	struct file_system_type *type;
	struct vfsmount *mnt;
	int flags = 0;

	type = get_fs_type(lustre_fs);
	if (!type)
		return ERR_PTR(-ENODEV);
	mnt = vfs_kern_mount(type, flags, dev, opt);
	module_put(type->owner);

	return mnt;
}

static int lustre_cli_destroy(struct md_private *lp)
{
	return generic_cli_destroy(lp);
}

#define LUSTRE_FS "%s:/%s"
#define LUSTRE_OPT "device=%s"

static int lustre_cli_create(struct md_private **cli, const void __user *data)
{
	struct md_private *ret = NULL;
	const struct lustre_mnt_opt *mnt = data;
	char *opt = NULL;
	char *fs = NULL;
	char *fsname;
	char *dstnid;
	int rc;

	fsname = getname(mnt->fsname);
	dstnid = getname(mnt->dstnid);
	if (!fsname || !dstnid) {
		rc = -ENOMEM;
		goto out_name;
	}

	rc = generic_cli_create(&ret);
	if (rc < 0)
		goto out_name;

	rc = snprintf(NULL, 0, LUSTRE_FS, dstnid, fsname) + 1;
	fs = kmalloc(rc, GFP_KERNEL);
	if (fs == NULL) {
		rc = -ENOMEM;
		goto error;
	}

	opt = kmalloc(rc + sizeof(LUSTRE_OPT), GFP_KERNEL);
	if (opt == NULL) {
		rc = -ENOMEM;
		goto error;
	}

	snprintf(fs, rc, LUSTRE_FS, dstnid, fsname);
	snprintf(opt, rc + sizeof(LUSTRE_OPT), LUSTRE_OPT, fs);
	DPRINT("dev %s / opt %s\n", fs, opt);

	ret->lp_mnt = mount_lustre(fs, opt);
	if (IS_ERR(ret->lp_mnt)) {
		rc = PTR_ERR(ret->lp_mnt);
		ret->lp_mnt = NULL;
		goto error;
	}

	ret->lp_root = dget(ret->lp_mnt->mnt_sb->s_root);

	*cli = ret;
	rc = 0;
	ret = NULL;
error:
	if (ret)
		generic_cli_destroy(ret);
	if (opt)
		kfree(opt);
	if (fs)
		kfree(fs);
out_name:
	if (dstnid)
		putname(dstnid);
	if (fsname)
		putname(fsname);
	return rc;
}

struct md_client md_lustre_cli = {
	.cli_init 	= lustre_cli_create,
	.cli_prerun	= generic_cli_prerun,
	.cli_fini	= lustre_cli_destroy,

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
