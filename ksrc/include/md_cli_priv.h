#ifndef _MD_CLI_PRIV_H_
#define _MD_CLI_PRIV_H_

enum {
	MAX_OPEN_HANDLES = 1000
};

struct md_private {
	struct vfsmount		*lp_mnt;
	struct file		**lp_open;
};

int generic_cli_create(struct md_private **lp);
int generic_cli_destroy(struct md_private *lp);

int generic_cli_create(struct md_private **lp);
int generic_cli_destroy(struct md_private *lp);
int generic_cli_prerun(struct md_private *cli);
int generic_cli_cd(struct md_private *cli, const char *pwd);
int generic_cli_mkdir(struct md_private *cli, const char *pwd, const int mode);
int generic_cli_readdir(struct md_private *cli, const char *name);
int generic_cli_unlink(struct md_private *cli, const char *name);
int generic_cli_open(struct md_private *lp, const char *name, 
		     const long flags, const long mode, const long reg);
int generic_cli_close(struct md_private *lp, const long reg);
int generic_cli_stat(struct md_private *cli, const char *name);
int generic_cli_setattr(const char *name, struct iattr *newattrs);
int generic_cli_chown(struct md_private *cli, const char *name,
		      long uid, long gid);
int generic_cli_chmod(struct md_private *cli, const char *name, long mode);
int generic_cli_chtime(struct md_private *cli, const char *name, long time);
int generic_cli_truncate(struct md_private *cli, const char *name, long size);
int generic_cli_lookup(struct md_private *cli, const char *name);
int generic_cli_softlink(struct md_private *cli, const char *name,
			 const char *linkname);
int generic_cli_hardlink(struct md_private *cli, const char *name,
			 const char *linkname);
int generic_cli_readlink(struct md_private *cli, const char *linkname);
int generic_cli_rename(struct md_private *cli, const char *oldname,
		       const char *newname);

#endif
