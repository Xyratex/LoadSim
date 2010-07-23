#include <linux/slab.h>
#include <linux/errno.h>

#include "clients/client.h"

struct lustre_private {
	struct vfsmount		*lp_mnt;
};

const static char lustre_fs[] = "llite";

static struct vfsmount *mount_lustre(char *opts)
{
#if 0
	struct file_system_type *type;
	struct vfsmount *mnt;

	type = get_fs_type(lustre_fs);
	if (!type)
		return ERR_PTR(-ENODEV);
	mnt = kern_mount(type, flags, name);
	put_filesystem(type);

	return mnt;
#endif
	return NULL;
}

int lustre_md_create(struct md_client *cli, char *fsname, char *dstnid)
{
	struct lustre_private *ret;

	ret = kmalloc(sizeof *ret, GFP_KERNEL);
	if (ret == NULL)
		return -ENOMEM;

	ret->lp_mnt = mount_lustre(NULL);

	cli->private = ret;

	return 0;
}

int lustre_md_destroy(struct md_client *cli)
{
	struct lustre_private *lp = cli->private;


	return 0;
}

int lustre_md_cd(struct md_client *cli, const char *pwd)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_mkdir(struct md_client *cli, const char *pwd)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_readdir(struct md_client *cli, const char *pwd)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_unlink(struct md_client *cli, const char *name)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_open(struct md_client *cli, const char *name, 
		      const long flags, const long reg)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_close(struct md_client *cli, const long reg)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_stat(struct md_client *cli, const char *name)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_setattr(struct md_client *cli, const char *name, const long attr)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_lookup(struct md_client *cli, const char *name)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_softlink(struct md_client *cli, const char *name,
			  const char *linkname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_hardlink(struct md_client *cli, const char *name,
			  const char *linkname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_readlink(struct md_client *cli,  const char *linkname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int lustre_md_rename(struct md_client *cli, const char *oldname,
			  const char *newname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

struct md_client lustre_cli = {
	.cli_init 	= lustre_md_create,
	.cli_fini	= lustre_md_destroy,

	.cd		= lustre_md_cd,
	.mkdir		= lustre_md_mkdir,
	.readdir	= lustre_md_readdir,
	.unlink		= lustre_md_unlink,
	.open		= lustre_md_open,
	.close		= lustre_md_close,
	.stat		= lustre_md_stat,
	.setattr	= lustre_md_setattr,
	.lookup		= lustre_md_lookup,
	.softlink	= lustre_md_softlink,
	.hardlink	= lustre_md_hardlink,
	.readlink	= lustre_md_readlink,
	.rename		= lustre_md_rename,
};
