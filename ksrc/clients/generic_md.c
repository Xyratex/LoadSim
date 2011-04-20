#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/namei.h>
#include <linux/mount.h>
#include <linux/sched.h>
#include <linux/statfs.h>
#include <linux/module.h>

#include "kdebug.h"
#include "compat.h"
#include "clients/client.h"
#include "clients/md_cli_priv.h"

int generic_cli_create(struct md_private **lp)
{
	struct md_private *ret;
	int rc;

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
	*lp = ret;
	return 0;
error:
	if (ret != NULL)
		kfree(ret);

	return rc;
}

int generic_cli_destroy(struct md_private *lp)
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

int generic_cli_prerun(struct md_private *cli)
{
	struct fs_struct old_fs;

	/* make lustre mount as pwd, root, altroot */
	write_lock(&current->fs->lock);
	old_fs = *current->fs;
	
	sim_fs_pwdmnt(current->fs) = mntget(cli->lp_mnt);
	sim_fs_pwd(current->fs) = dget(cli->lp_mnt->mnt_sb->s_root);
	sim_fs_rootmnt(current->fs) = mntget(cli->lp_mnt);
	sim_fs_root(current->fs) = dget(cli->lp_mnt->mnt_sb->s_root);
#ifdef HAVE_VFS_ALTROOT
	current->fs->altrootmnt = mntget(cli->lp_mnt);
	current->fs->altroot = dget(cli->lp_mnt->mnt_sb->s_root);
#endif
	write_unlock(&current->fs->lock);

	DPRINT("root %p/%p\n", sim_fs_pwd(current->fs), sim_fs_pwd(current->fs)->d_inode);

	if (sim_fs_pwd(&old_fs))
		dput(sim_fs_pwd(&old_fs));
	if (sim_fs_pwdmnt(&old_fs))
		mntput(sim_fs_pwdmnt(&old_fs));

	if (sim_fs_root(&old_fs))
		dput(sim_fs_root(&old_fs));

	if (sim_fs_rootmnt(&old_fs))
		mntput(sim_fs_rootmnt(&old_fs));

#ifdef HAVE_VFS_ALTROOT
	if (old_fs.altroot)
		dput(old_fs.altroot);
	if (old_fs.altrootmnt)
		mntput(old_fs.altrootmnt);
#endif
	return 0;
}

int generic_cli_cd(struct md_private *cli, const char *pwd)
{
	int retval;
	struct dentry *old_pwd;
	struct nameidata nd;

	ENTER();
	
	retval = path_lookup(pwd, LOOKUP_DIRECTORY | LOOKUP_FOLLOW | LOOKUP_NOALT, &nd);
	if (retval)
		goto out;

	retval = sim_exec_permission(nd);
	if (retval == 0) {
		/* copy of set_fs_pwd */
		write_lock(&current->fs->lock);
		old_pwd = sim_fs_pwd(current->fs);
		sim_fs_pwd(current->fs) = dget(sim_nd_dentry(nd));
		write_unlock(&current->fs->lock);

		if (old_pwd)
			dput(old_pwd);
	}
	sim_path_put(&nd);
out:
	DPRINT("cd leave %d\n", retval);
	return retval;
}

int generic_cli_mkdir(struct md_private *cli, const char *pwd, const int mode)
{
	int retval;
	struct nameidata nd;
	struct dentry *dir;

	ENTER();
	retval = path_lookup(pwd, LOOKUP_DIRECTORY | LOOKUP_PARENT, &nd);
	if (retval)
		goto out;

	DPRINT("found parent %p/%p\n", sim_nd_dentry(nd), sim_nd_dentry(nd)->d_inode);
	dir = lookup_create(&nd, 0); /* hold i_mutex */
	if (!IS_ERR(dir)) {
		retval = vfs_mkdir(sim_nd_dentry(nd)->d_inode, dir, mode);
		dput(dir);
	}
	mutex_unlock(&sim_nd_dentry(nd)->d_inode->i_mutex);
	sim_path_put(&nd);
out:
	DPRINT("mkdir leave %d\n", retval);
	return retval;
}

static int null_cb(void * __buf, const char * name, int namlen, loff_t offset,
		      u64 ino, unsigned int d_type)
{
	return 0;
}

int generic_cli_readdir(struct md_private *cli, const char *name)
{
	struct file *dir;
	int retval;

	dir = filp_open(name, O_DIRECTORY, 0000);
	if (IS_ERR(dir))
		return PTR_ERR(dir);
	DPRINT("readdir open finished\n");

	retval = vfs_readdir(dir, null_cb, NULL);

	filp_close(dir, 0);

	return 0;
}

int generic_cli_unlink(struct md_private *cli, const char *name)
{
	int retval;
	struct nameidata nd;
	struct dentry *child;

	retval = path_lookup(name, LOOKUP_DIRECTORY | LOOKUP_PARENT, &nd);
	if (retval)
		return retval;

	mutex_lock(&sim_nd_dentry(nd)->d_inode->i_mutex);
	child = lookup_one_len(nd.last.name, sim_nd_dentry(nd), strlen(nd.last.name));
	retval = PTR_ERR(child);
	if (!IS_ERR(child)) {
		if (child->d_inode != NULL) {
			if (S_ISDIR(child->d_inode->i_mode)) {
				retval = vfs_rmdir(sim_nd_dentry(nd)->d_inode, child);
				dput(child);
			} else {
				retval = vfs_unlink(sim_nd_dentry(nd)->d_inode, child);
				if (!retval && !(child->d_flags & DCACHE_NFSFS_RENAMED))
					d_delete(child);
			}
		} else {
			retval = -ENOENT;
		}
	}
	mutex_unlock(&sim_nd_dentry(nd)->d_inode->i_mutex);

	sim_path_put(&nd);
	return retval;
}

int generic_cli_open(struct md_private *lp, const char *name, 
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

int generic_cli_close(struct md_private *lp, const long reg)
{
	int rc;

	if (lp->lp_open[reg] != NULL)
		rc = filp_close(lp->lp_open[reg], 0);
	else
		rc = -EBADFD;
	return rc;
}

int generic_cli_stat(struct md_private *cli, const char *name)
{
	int retval;
	struct nameidata nd;
	struct kstat tmp;

	retval = path_lookup(name, 0, &nd);
	if (retval)
		goto out;
	retval = vfs_getattr(sim_nd_mnt(nd), sim_nd_dentry(nd), &tmp);
	sim_path_put(&nd);
out:
	DPRINT("stat rc %d\n", retval);
	return retval;
}

int generic_cli_setattr(const char *name, struct iattr *newattrs)
{
	int retval;
	int suid;
	struct nameidata nd;

	retval = path_lookup(name, 0, &nd);
	if (retval)
		return retval;

	retval = sim_write_permission(nd);
	if (retval)
		goto exit;

#if 0
	/* Remove suid/sgid if need */
	suid = should_remove_suid(sim_nd_dentry(nd));
	if (suid)
		newattrs->ia_valid |= ATTR_FORCE | suid;
#endif

	mutex_lock(&sim_nd_dentry(nd)->d_inode->i_mutex);
	retval = notify_change(sim_nd_dentry(nd), newattrs);
	mutex_unlock(&sim_nd_dentry(nd)->d_inode->i_mutex);
exit:
	sim_path_put(&nd);

	return retval;
}

int generic_cli_chown(struct md_private *cli, const char *name,
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

int generic_cli_chmod(struct md_private *cli, const char *name, long mode)
{
	struct iattr newattrs;

	newattrs.ia_mode = (mode & S_IALLUGO);
	newattrs.ia_valid = ATTR_MODE | ATTR_CTIME;

	return generic_cli_setattr(name, &newattrs);
}

int generic_cli_chtime(struct md_private *cli, const char *name, long time)
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

int generic_cli_truncate(struct md_private *cli, const char *name, long size)
{
	struct iattr newattrs;

	newattrs.ia_size = size;
	newattrs.ia_valid = ATTR_SIZE | ATTR_CTIME;

	return generic_cli_setattr(name, &newattrs);
}

int generic_cli_lookup(struct md_private *cli, const char *name)
{
	int retval;
	struct nameidata nd;

	retval = path_lookup(name, 0, &nd);
	if (retval)
		return retval;
	sim_path_put(&nd);

	return 0;
}

int generic_cli_softlink(struct md_private *cli, const char *name,
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
		retval = sim_vfs_symlink(sim_nd_dentry(nd)->d_inode, new, name, S_IALLUGO);
		dput(new);
	}
	mutex_unlock(&sim_nd_dentry(nd)->d_inode->i_mutex); // lookup_create
	sim_path_put(&nd);

	DPRINT("softlink return %d\n", retval);
	return retval;
}

int generic_cli_hardlink(struct md_private *cli, const char *name,
			 const char *linkname)
{
	int retval;
	struct nameidata ndold;
	struct nameidata ndnew;
	struct dentry *new;

	retval = path_lookup(name, 0, &ndold);
	if (retval) {
		return retval;
	}

	retval = path_lookup(linkname, LOOKUP_DIRECTORY | LOOKUP_PARENT, &ndnew);
	if (retval) {
		goto exit1;
	}

	new = lookup_create(&ndnew, 0);
	if (!IS_ERR(new)) {
		retval = vfs_link(sim_nd_dentry(ndold), 
				  sim_nd_dentry(ndnew)->d_inode, new);
		mutex_unlock(&sim_nd_dentry(ndnew)->d_inode->i_mutex); // lookup_create
		dput(new);
	} else {
		retval = PTR_ERR(new);
	}
	sim_path_put(&ndnew);
exit1:
	sim_path_put(&ndold);
	DPRINT("hardlink return %d\n", retval);
	return retval;
}

int generic_cli_readlink(struct md_private *cli, const char *linkname)
{
	int retval;
	struct nameidata nd;

	retval = path_lookup(linkname, LOOKUP_FOLLOW, &nd);
	if (retval)
		return retval;
	sim_path_put(&nd);

	DPRINT("readlink %d\n", retval);
	return retval;
}

int generic_cli_rename(struct md_private *cli, const char *oldname,
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

	p = lock_rename(sim_nd_dentry(ndold), sim_nd_dentry(ndnew));
	old = lookup_one_len(ndold.last.name, sim_nd_dentry(ndold), strlen(ndold.last.name));
	if (IS_ERR(old)) {
		retval = PTR_ERR(old);
		goto exit2;
	}

	new = lookup_one_len(ndnew.last.name, sim_nd_dentry(ndnew), strlen(ndnew.last.name));
	if (IS_ERR(new)) {
		retval = PTR_ERR(new);
		goto exit3;
	}
	if ((new == p) || (old == p)) {
		retval = -EINVAL;
		goto exit4;
	}

	retval = vfs_rename(sim_nd_dentry(ndold)->d_inode, old,
			    sim_nd_dentry(ndnew)->d_inode, new);
exit4:
	dput(new);
exit3:
	dput(old);
exit2:
	unlock_rename(sim_nd_dentry(ndold), sim_nd_dentry(ndnew));
	sim_path_put(&ndnew);
exit1:
	sim_path_put(&ndold);
	return retval;
}
