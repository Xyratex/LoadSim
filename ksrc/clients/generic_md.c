#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/statfs.h>
#include <linux/module.h>

#include "kdebug.h"
#include "clients/client.h"

struct lustre_private {
	struct vfsmount		*lp_mnt;
};

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

int generic_cli_destroy(struct md_client *cli)
{
	struct lustre_private *lp = cli->private;

	if (lp == NULL)
		return 0;

	if (lp->lp_mnt)
		mntput(lp->lp_mnt);

	kfree(lp);

	return 0;
}

#define LUSTRE_FS "%s:/%s"
#define LUSTRE_OPT "device=%s"

int generic_cli_create(struct md_client *cli, char *fsname, char *dstnid)
{
	struct lustre_private *ret;
	char *fs = NULL;
	char *opt = NULL;
	struct fs_struct old_fs;
	int rc;

	cli->private = NULL;

	rc = snprintf(NULL, 0, LUSTRE_FS, dstnid, fsname) + 1;
	fs = kmalloc(rc, GFP_KERNEL);
	if (fs == NULL)
		return -ENOMEM;

	opt = kmalloc(rc + sizeof(LUSTRE_OPT), GFP_KERNEL);
	if (opt == NULL) {
		rc = -ENOMEM;
		goto error;
	}

	ret = kmalloc(sizeof *ret, GFP_KERNEL);
	if (ret == NULL) {
		rc = -ENOMEM;
		goto error;
	}

	snprintf(fs, rc, LUSTRE_FS, dstnid, fsname);
	snprintf(opt, rc + sizeof(LUSTRE_OPT), LUSTRE_OPT, fs);
	DPRINT("dev %s / opt %s\n", fs, opt);

	ret->lp_mnt = mount_lustre(fs, opt);
	if (IS_ERR(ret->lp_mnt)) {
		rc = PTR_ERR(ret->lp_mnt);
		goto error;
	}

	/* make lustre mount as pwd, root, altroot */
	write_lock(&current->fs->lock);
	old_fs = *current->fs;
	current->fs->pwdmnt = mntget(ret->lp_mnt);
	current->fs->pwd = dget(ret->lp_mnt->mnt_sb->s_root);
	current->fs->rootmnt = mntget(ret->lp_mnt);
	current->fs->root = dget(ret->lp_mnt->mnt_sb->s_root);
	current->fs->altrootmnt = mntget(ret->lp_mnt);
	current->fs->altroot = dget(ret->lp_mnt->mnt_sb->s_root);
	write_unlock(&current->fs->lock);

	DPRINT("root %p/%p\n", current->fs->pwd, current->fs->pwd->d_inode);

	if (old_fs.pwd)
		dput(old_fs.pwd);
	if (old_fs.root)
		dput(old_fs.root);
	if (old_fs.altroot)
		dput(old_fs.altroot);

	if (old_fs.pwdmnt)
		mntput(old_fs.pwdmnt);
	if (old_fs.rootmnt)
		mntput(old_fs.rootmnt);
	if (old_fs.altrootmnt)
		mntput(old_fs.altrootmnt);

	cli->private = ret;
	kfree(fs);

	return 0;
error:
	if (ret)
		kfree(ret);
	if (opt)
		kfree(opt);
	kfree(fs);
	return rc;
}



int generic_cli_cd(struct md_client *cli, const char *pwd)
{
	struct lustre_private *lp = cli->private;
	int retval;
	struct dentry *old_pwd;
	struct vfsmount *old_pwdmnt = NULL;
	struct nameidata nd;

	ENTER();
	
	retval = path_lookup(pwd, LOOKUP_DIRECTORY | LOOKUP_FOLLOW | LOOKUP_NOALT, &nd);
	if (retval)
		goto out;

	retval = vfs_permission(&nd, MAY_EXEC);
	if (retval == 0) {
		/* copy of set_fs_pwd */
		write_lock(&current->fs->lock);
		old_pwd = current->fs->pwd;
		current->fs->pwd = dget(nd.dentry);
		write_unlock(&current->fs->lock);

		if (old_pwd)
			dput(old_pwd);
		if (old_pwdmnt)
			mntput(old_pwdmnt);
	}
	path_release(&nd);
out:
	DPRINT("cd leave %d\n", retval);
	return retval;
}

int generic_cli_mkdir(struct md_client *cli, const char *pwd)
{
	struct lustre_private *lp = cli->private;
	int retval;
	struct nameidata nd;
	struct dentry *dir;
	long	mode = 0666;

	ENTER();
	retval = path_lookup(pwd, LOOKUP_DIRECTORY | LOOKUP_PARENT, &nd);
	if (retval)
		goto out;

	DPRINT("found parent %p/%p\n", nd.dentry, nd.dentry->d_inode);
	dir = lookup_create(&nd, 0); /* hold i_mutex */
	if (!IS_ERR(dir)) {
		retval = vfs_mkdir(nd.dentry->d_inode, dir, mode);
	}
	mutex_unlock(&nd.dentry->d_inode->i_mutex);
	path_release(&nd);
out:
	DPRINT("mkdir leave %d\n", retval);
	return retval;
}

int generic_cli_readdir(struct md_client *cli, const char *pwd)
{
	struct lustre_private *lp = cli->private;


	return 0;
}

int generic_cli_unlink(struct md_client *cli, const char *name)
{
	struct lustre_private *lp = cli->private;
	int retval;
	struct nameidata nd;
	struct dentry *child;

	retval = path_lookup(name, LOOKUP_DIRECTORY | LOOKUP_PARENT, &nd);
	if (retval)
		return retval;

	mutex_lock(&nd.dentry->d_inode->i_mutex);
	child = lookup_one_len(nd.last.name, nd.dentry, strlen(name));
	retval = PTR_ERR(child);
	if (!IS_ERR(child)) {
		if (child->d_inode != NULL)
			retval = vfs_unlink(nd.dentry->d_inode, child);
		else
			retval = -ENOENT;
	}
	mutex_unlock(&nd.dentry->d_inode->i_mutex);

	path_release(&nd);
	return retval;
}

int generic_cli_open(struct md_client *cli, const char *name, 
		      const long flags, const long reg)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int generic_cli_close(struct md_client *cli, const long reg)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int generic_cli_stat(struct md_client *cli, const char *name)
{
	struct lustre_private *lp = cli->private;
	int retval;
	struct nameidata nd;
	struct kstatfs tmp;

	retval = path_lookup(name, LOOKUP_DIRECTORY, &nd);
	if (retval)
		return retval;
	retval = vfs_statfs(nd.dentry, &tmp);
	path_release(&nd);

	return retval;
}

int generic_cli_setattr(struct md_client *cli, const char *name, const long attr)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int generic_cli_lookup(struct md_client *cli, const char *name)
{
	struct lustre_private *lp = cli->private;
	int retval;
	struct nameidata nd;

	retval = path_lookup(name, LOOKUP_DIRECTORY, &nd);
	if (retval)
		return retval;
	path_release(&nd);

	return 0;
}

int generic_cli_softlink(struct md_client *cli, const char *name,
			  const char *linkname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int generic_cli_hardlink(struct md_client *cli, const char *name,
			  const char *linkname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int generic_cli_readlink(struct md_client *cli,  const char *linkname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

int generic_cli_rename(struct md_client *cli, const char *oldname,
			  const char *newname)
{
	struct lustre_private *lp = cli->private;

	return 0;
}

struct md_client generic_cli = {
	.cli_init 	= generic_cli_create,
	.cli_fini	= generic_cli_destroy,

	.cd		= generic_cli_cd,
	.mkdir		= generic_cli_mkdir,
	.readdir	= generic_cli_readdir,
	.unlink		= generic_cli_unlink,
	.open		= generic_cli_open,
	.close		= generic_cli_close,
	.stat		= generic_cli_stat,
	.setattr	= generic_cli_setattr,
	.lookup		= generic_cli_lookup,
	.softlink	= generic_cli_softlink,
	.hardlink	= generic_cli_hardlink,
	.readlink	= generic_cli_readlink,
	.rename		= generic_cli_rename,
};
