#ifndef __SIMUL_API_
#define __SIMUL_API_

struct server_link;
struct simul_res;

/**
 userland to kernel interface API, userland part.

 that code interact with kernel module and provide API
 to create test clients; upload, run programs, and obtain results.
*/


/**
 open and initialize test environment in kernel
 */
int simul_api_open(void);

/**
 create one test client and upload on it.

 */
int simul_api_cli_create(char *cliname, long cliid, struct server_link *sl,
			 void *data, int size);

/**
 run programs on all clients
 */
int simul_api_run(void);
 
/**
 waiting until all programs is finished
 */
int simul_api_wait_finished(void);

/**
 obtain results from finished clients
 */
int simul_api_get_results(struct simul_res *res);

/**
 destroy kernel test environment and close kernel<>userland communication
 */
int simul_api_close(void);

#endif
