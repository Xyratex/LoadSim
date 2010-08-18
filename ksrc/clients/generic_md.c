#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/statfs.h>
#include <linux/module.h>

#include "kdebug.h"
#include "clients/client.h"

struct md_private {
	struct vfsmount		*lp_mnt;
	struct file		**lp_open;
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

static int generic_cli_destroy(struct md_private *lp)
{
	if (lp == NULL)
		return 0;

	if (lp->lp_open) {
		/* XXX close all files */
		kfree(lp->lp_open);
	}

	if (lp->lp_mnt) {
		DPRINT("mntput \n");
		mntput(lp->lp_mnt);
	}

	kfree(lp);

	return 0;
}

#define LUSTRE_FS "%s:/%s"
#define LUSTRE_OPT "device=%s"

static int generic_cli_create(struct md_private **cli, const char *fsname, const char *dstnid)
{
	struct md_private *ret = NULL;
	char *fs = NULL;
	char *opt = NULL;
	int rc;

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
	memset(ret, 0, sizeof *ret);
	
	ret->lp_open = kmalloc(sizeof(struct file *) * MAX_OPEN_HANDLES, GFP_KERNEL);
	if (ret->lp_open == NULL) {
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
	*cli = ret;
	rc = 0;
	ret = NULL;
error:
	if (ret)
		generic_cli_destroy(ret);
	if (opt)
		kfree(opt);
	kfree(fs);
	return rc;
}

static int generic_cli_prerun(struct md_private *cli)
{
	struct fs_struct old_fs;

	/* make lustre mount as pwd, root, altroot */
	write_lock(&current->fs->lock);
	old_fs = *current->fs;
	current->fs->pwdmnt = mntget(cli->lp_mnt);
	current->fs->pwd = dget(cli->lp_mnt->mnt_sb->s_root);
	current->fs->rootmnt = mntget(cli->lp_mnt);
	current->fs->root = dget(cli->lp_mnt->mnt_sb->s_root);
	current->fs->altrootmnt = mntget(cli->lp_mnt);
	current->fs->altroot = dget(cli->lp_mnt->mnt_sb->s_root);
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

	return 0;
}

static int generic_cli_cd(struct md_private *cli, const char *pwd)
{
	int retval;
	struct dentry *old_pwd;
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
	}
	path_release(&nd);
out:
	DPRINT("cd leave %d\n", retval);
	return retval;
}

static int generic_cli_mkdir(struct md_private *cli, const char *pwd, const int mode)
{
	int retval;
	struct nameidata nd;
	struct dentry *dir;

	ENTER();
	retval = path_lookup(pwd, LOOKUP_DIRECTORY | LOOKUP_PARENT, &nd);
	if (retval)
		goto out;

	DPRINT("found parent %p/%p\n", nd.dentry, nd.dentry->d_inode);
	dir = lookup_create(&nd, 0); /* hold i_mutex */
	if (!IS_ERR(dir)) {
		retval = vfs_mkdir(nd.dentry->d_inode, dir, mode);
		dput(dir);
	}
	mutex_unlock(&nd.dentry->d_inode->i_mutex);
	path_release(&nd);
out:
	DPRINT("mkdir leave %d\n", retval);
	return retval;
}

static int null_cb(void * __buf, const char * name, int namlen, loff_t offset,
		      u64 ino, unsigned int d_type)
{
	return 0;
}

static int generic_cli_readdir(struct md_private *cli, const char *name)
{
	struct file *dir;
	int retval;

	dir = filp_open(name, O_DIRECTORY, 0000);
	if (IS_ERR(dir))
		return PTR_ERR(dir);
	DPRINT("open finished\n");

	retval = vfs_readdir(dir, null_cb, NULL);

	filp_close(dir, 0);

	return 0;
}

static int generic_cli_unlink(struct md_private *cli, const char *name)
{
	int retval;
	struct nameidata nd;
	struct dentry *child;

	retval = path_lookup(name, LOOKUP_DIRECTORY | LOOKUP_PARENT, &nd);
	if (retval)
		return retval;

	mutex_lock(&nd.dentry->d_inode->i_mutex);
	child = lookup_one_len(nd.last.name, nd.dentry, strlen(nd.last.name));
	retval = PTR_ERR(child);
	if (!IS_ERR(child)) {
		if (child->d_inode != NULL) {
			if (S_ISDIR(child->d_inode->i_mode)) {
				retval = vfs_rmdir(nd.dentry->d_inode, child);
				dput(child);
			} else {
				retval = vfs_unlink(nd.dentry->d_inode, child);
				if (!retval && !(child->d_flags & DCACHE_NFSFS_RENAMED))
					d_delete(child);
			}
		} else {
			retval = -ENOENT;
		}
	}
	mutex_unlock(&nd.dentry->d_inode->i_mutex);

	path_release(&nd);
	return retval;
}

static int generic_cli_open(struct md_private *lp, const char *name, 
			const long flags, const long mode, const long reg)
{
	int rc = 0;

	lp->lp_open[reg] = filp_open(name, flags, mode);
	if (IS_ERR(lp->lp_open[reg])) {
		rc = PTR_ERR(lp->lp_open[reg]);
		lp->lp_open[reg] = NULL;
	}
	DPRINT("open finished %d\n", rc);
	return rc;
}

static int generic_cli_close(struct md_private *lp, const long reg)
{
	int rc;

	if (lp->lp_open[reg] != NULL)
		rc = filp_close(lp->lp_open[reg], 0);
	else
		rc = -EBADFD;
	return rc;
}

static int generic_cli_stat(struct md_private *cli, const char *name)
{
	int retval;
	struct nameidata nd;
	struct kstat tmp;

	retval = path_lookup(name, 0, &nd);
	if (retval)
		goto out;
	retval = vfs_getattr(nd.mnt, nd.dentry, &tmp);
	path_release(&nd);
out:
	DPRINT("stat rc %d\n", retval);
	return retval;
}

static int generic_cli_setattr(const char *name, struct iattr *newattrs)
{
	int retval;
	struct nameidata nd;

	retval = path_lookup(name, 0, &nd);
	if (retval)
		return retval;

	retval = vfs_permission(&nd, MAY_WRITE);
	if (retval)
		goto exit;

	/* Remove suid/sgid if need */
	remove_suid(nd.dentry);

	mutex_lock(&nd.dentry->d_inode->i_mutex);
	retval = notify_change(nd.dentry, newattrs);
	mutex_unlock(&nd.dentry->d_inode->i_mutex);
exit:
	path_release(&nd);

	return retval;
}

static int generic_cli_chown(struct md_private *cli, const char *name,
			    long uid, long gid)
{
	struct iattr newattrs;

	newattrs.ia_valid =  ATTR_CTIME;
	if (uid != -1) {
		newattrs.ia_valid |= ATTR_UID;
		newattrs.ia_uid = uid;
	}
	if (gid != -1) {
		newattrs.ia_valid |= ATTR_GID;
		newattrs.ia_gid = gid;
	}

	return generic_cli_setattr(name, &newattrs);
}

static int generic_cli_chmod(struct md_private *cli, const char *name, long mode)
{
	struct iattr newattrs;

	newattrs.ia_mode = (mode & S_IALLUGO);
	newattrs.ia_valid = ATTR_MODE | ATTR_CTIME;

	return generic_cli_setattr(name, &newattrs);
}

static int generic_cli_chtime(struct md_private *cli, const char *name, long time)
{
	struct iattr newattrs;

	newattrs.ia_valid = ATTR_CTIME | ATTR_MTIME | ATTR_ATIME;
	if (time != 0) {
		newattrs.ia_atime.tv_sec = time;
		newattrs.ia_atime.tv_nsec = 0;
	} else {
		newattrs.ia_atime = current_kernel_time();
	}
	newattrs.ia_mtime = newattrs.ia_ctime = newattrs.ia_atime;

	return generic_cli_setattr(name, &newattrs);
}

static int generic_cli_truncate(struct md_private *cli, const char *name, long size)
{
	struct iattr newattrs;

	newattrs.ia_size = size;
	newattrs.ia_valid = ATTR_SIZE | ATTR_CTIME;

	return generic_cli_setattr(name, &newattrs);
}


static int generic_cli_lookup(struct md_private *cli, const char *name)
{
	int retval;
	struct nameidata nd;

	retval = path_lookup(name, 0, &nd);
	if (retval)
		return retval;
	path_release(&nd);

	return 0;
}

static int generic_cli_softlink(struct md_private *cli, const char *name,
			  const char *linkname)
{
	int retval;
	struct nameidata nd;
	struct dentry *new;

	DPRINT("sl %s/%s\n", linkname, name);
	retval = path_lookup(linkname, LOOKUP_PARENT, &nd);
	if (retval)
		return retval;

	new = lookup_create(&nd, 0);
	retval = PTR_ERR(new);
	if (!IS_ERR(new)) {
		retval = vfs_symlink(nd.dentry->d_inode, new, name, S_IALLUGO);
		dput(new);
	}
	mutex_unlock(&nd.dentry->d_inode->i_mutex); // lookup_create
	path_release(&nd);

	DPRINT("softlink return %d\n", retval);
	return retval;
}

static int generic_cli_hardlink(struct md_private *cli, const char *name,
			  const char *linkname)
{
	int retval;
	struct nameidata nd;
	struct dentry *old;
	struct dentry *new;

	retval = path_lookup(name, LOOKUP_DIRECTORY | LOOKUP_PARENT, &nd);
	if (retval)
		return retval;

	old = lookup_one_len(nd.last.name, nd.dentry, strlen(nd.last.name));
	if (IS_ERR(old)) {
		retval = PTR_ERR(old);
		goto exit1;
	}

	new = lookup_create(&nd, 0);
	retval = PTR_ERR(new);
	if (!IS_ERR(new)) {
		retval = vfs_link(old, nd.dentry->d_inode, new);
		dput(new);
	}
	mutex_unlock(&nd.dentry->d_inode->i_mutex); // lookup_create
	dput(old);
exit1:
	DPRINT("hardlink return %d\n", retval);
	path_release(&nd);
	return retval;
}

static int generic_cli_readlink(struct md_private *cli, const char *linkname)
{
	int retval;
	struct nameidata nd;

	retval = path_lookup(linkname, LOOKUP_FOLLOW, &nd);
	if (retval)
		return retval;
	path_release(&nd);

	DPRINT("readlink %d\n", retval);
	return retval;
}

static int generic_cli_rename(struct md_private *cli, const char *oldname,
			  const char *newname)
{
	int retval;
	struct nameidata ndold;
	struct nameidata ndnew;
	struct dentry *p;
	struct dentry *old;
	struct dentry *new;

	retval = path_lookup(oldname, LOOKUP_DIRECTORY | LOOKUP_PARENT, &ndold);
	if (retval)
		return retval;

	retval = path_lookup(newname, LOOKUP_DIRECTORY | LOOKUP_PARENT, &ndnew);
	if (retval)
		goto exit1;

	p = lock_rename(ndold.dentry, ndnew.dentry);
	old = lookup_one_len(ndold.last.name, ndold.dentry, strlen(ndold.last.name));
	if (IS_ERR(old)) {
		retval = PTR_ERR(old);
		goto exit2;
	}

	new = lookup_one_len(ndnew.last.name, ndnew.dentry, strlen(ndnew.last.name));
	if (IS_ERR(new)) {
		retval = PTR_ERR(new);
		goto exit3;
	}
	if ((new == p) || (old == p)) {
		retval = -EINVAL;
		goto exit4;
	}

	retval = vfs_rename(ndold.dentry->d_inode, old,
			    ndnew.dentry->d_inode, new);
exit4:
	dput(new);
exit3:
	dput(old);
exit2:
	unlock_rename(ndold.dentry, ndnew.dentry);
	path_release(&ndnew);
exit1:
	path_release(&ndold);
	return retval;
}

struct md_client generic_cli = {
	.cli_init 	= generic_cli_create,
	.cli_prerun	= generic_cli_prerun,
	.cli_fini	= generic_cli_destroy,

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
