#ifndef __SIMUL_ENV_H_
#define __SIMUL_ENV_H_

struct stack_vm;
struct simul_ioctl_cli;
struct simul_res;

struct md_client;
struct md_private;

#include <linux/list.h>

/**
 environment for tests
 
 */
struct simul_env {
	/**
	 link into global clients list
	*/
	struct list_head	se_link;
	/**
	 client name (same as client UUID)
	 */
	char			*se_name;
	/**
	 client identifier on userland
	 */
	int			se_id;
	/**
	 virtual machine attached to the client
	*/
	struct stack_vm		*se_vm;
	/**
	 private data
	 */
	union {
		struct md_client	*se_md;
	} u;
	struct md_private	*se_data;
};

/**
 create one test enviroenment and connect into global list
 */
int env_create(struct simul_env **env, struct simul_ioctl_cli *data);

/**
 destroy all clients in global list
 */
void env_destroy_all(void);

/**
 return number of enviroments created
 */
int env_count(void);

/**
 get results
 */
void env_results_get(struct simul_res *data);

#endif
