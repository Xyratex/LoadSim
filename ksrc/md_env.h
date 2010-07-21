#ifndef __MD_ENV_H_
#define __MD_ENV_H_

struct dentry;
struct obd_export;
struct stack_vm;
struct simul_ioctl_cli;

#include <linux/list.h>

/**
 environment for metata tests
 
 each client has open environment where stored a private data.
 */
struct md_env {
	/**
	 link into global clients list
	*/
	struct list_head	mde_link;
	/**
	 client name (same as client UUID)
	 */
	char			*mde_name;
	/**
	 used to connect
	 */
	struct obd_export	*mde_mdc;
	/**
	 root dentry to start lookup
	 */
	struct dentry		*mde_root;
	/**
	 current directory
	 */
	struct dentry		*mde_pwd;
	/**
	 registers for an open calls
	 */
	void			*mde_open;
	/**
	 virtual machine attached to the client
	*/
	struct stack_vm		*mde_vm;
};

/**
 create one client and connect into global list
 */
int md_client_create(struct md_env **env, struct simul_ioctl_cli *data);

/**
 deallocate one client
*/
void md_client_destroy(struct md_env *env);

/**
 destroy all clients in global list
 */
void md_clients_destroy(void);

#endif
