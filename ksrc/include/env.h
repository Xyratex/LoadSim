#ifndef __SIMUL_ENV_H_
#define __SIMUL_ENV_H_

struct stack_vm;
struct simul_stat_op;
struct simul_ioctl_cli;

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

	uint32_t		se_stat_tn; /** << number of stats */
	struct op_time_stat	*se_stat_t;
};

/**
 create one test enviroenment and connect into global list
 */
int env_create(struct simul_env **env, struct simul_ioctl_cli *data);

int env_run(struct simul_env *env);

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
int env_results_get(uint32_t id, uint32_t __user *res, uint32_t __user *ip,
                    uint64_t __user *total_time,
		     struct simul_stat_op __user *data);

#endif
