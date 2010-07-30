#ifndef _SIMUL_CLIENTS_H_
#define _SIMUL_CLIENTS_H_

struct md_client {
	void *private;
	int (*cli_init)(struct md_client *env, char *fsname, char *dstnid);
	int (*cli_fini)(struct md_client *env);

	/**
	 change working directiry
	 \a name new work directory name
	 */
	int (*cd)(struct md_client *env, const char *name);
	/**
	 create a new directiry
	 \a name new work directory name
	 */
	int (*mkdir)(struct md_client *, const char *name);
	/**
	 */
	int (*readdir)(struct md_client *, const char *name);
	/**
	 */
	int (*unlink)(struct md_client *,const char *name);

	int (*open)(struct md_client *, const char *name,
		    const long flags, const long reg);
	int (*close)(struct md_client *, long reg);

	/**
	 */
	int (*stat)(struct md_client *,const char *name);
	/**
	 change attributes for md object
	 */
	int (*setattr)(struct md_client *, const char *name, long flags);
	/**
	 */
	int (*lookup)(struct md_client *,const char *name);
	/**
	 */
	int (*softlink)(struct md_client *, const char *oldname, const char *newname);
	int (*hardlink)(struct md_client *, const char *oldname, const char *newname);
	int (*readlink)(struct md_client *, const char *oldname);
	int (*rename)(struct md_client *,const char *oldname, const char *newname);
};

extern struct md_client generic_cli;

#endif
