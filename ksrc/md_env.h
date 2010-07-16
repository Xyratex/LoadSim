#ifndef __MD_ENV_H_
#define __MD_ENV_H_

/**
 enviroment for metata tests
 */
struct md_env {
	/**
	 used for raw operations
	 */
	struct obd_export	*mde_mdc;
	/**
	 root dentry to start lookup
	 */
	struct dentry		mde_root;
	/**
	 current directory
	 */
	struct dentry		mde_pwd;
};
#endif