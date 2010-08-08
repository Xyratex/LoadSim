#ifndef _SIMUL_CLIENTS_H_
#define _SIMUL_CLIENTS_H_

enum {
	MAX_OPEN_HANDLES = 1000
};

struct md_private;

struct md_client {
	int (*cli_init)(struct md_private **env, char *fsname, char *dstnid);
	int (*cli_prerun)(struct md_private *env);
	int (*cli_fini)(struct md_private *env);

	/**
	 change working directiry
	 \a name new work directory name
	 */
	int (*cd)(struct md_private *env, const char *name);
	/**
	 create a new directiry
	 \a name new work directory name
	 */
	int (*mkdir)(struct md_private *, const char *name, const int mode);
	/**
	 */
	int (*readdir)(struct md_private *, const char *name);
	/**
	 */
	int (*unlink)(struct md_private *,const char *name);

	int (*open)(struct md_private *, const char *name,
		    const long flags, const long mode, const long reg);
	int (*close)(struct md_private *, long reg);

	/**
	 */
	int (*stat)(struct md_private *,const char *name);
	/**
	 change attributes for md object
	 */
	int (*chmod)(struct md_private *, const char *name, long mode);
	int (*chown)(struct md_private *, const char *name, long uid, long gid);
	int (*chtime)(struct md_private *, const char *name, long time);
	int (*truncate)(struct md_private *, const char *name, long size);
	/**
	 */
	int (*lookup)(struct md_private *,const char *name);
	/**
	 */
	int (*softlink)(struct md_private *, const char *oldname, const char *newname);
	int (*hardlink)(struct md_private *, const char *oldname, const char *newname);
	int (*readlink)(struct md_private *, const char *oldname);
	int (*rename)(struct md_private *,const char *oldname, const char *newname);
};

extern struct md_client generic_cli;

#endif
