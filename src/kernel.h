#ifndef __SIMUL_API_
#define __SIMUL_API_

/**
 userland to kernel interface API, userland part.

 that code interact with kernel module and provide folling API
 to create test clients, upload and run programs, and obtain results.
*/


/**
 open and initialize test enviroment in kernel
 */
int simul_api_open(void);

/**
 create one test client and upload on it.

 */
int simul_api_cli_create(char *name, char *dstnid, void *data, int size)

/**
 run programs on all clients
 */
 
/**
 waiting until all programs is finished
 */
int simul_api_wait_finished(void);

/**
 obtain results from finished clients
 */
int simul_api_get_results(long *res);

/**
 destroy kernel test enviroment and close kernel<>userland communication
 */
int simul_api_close(void);

#endif