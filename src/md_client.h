#ifndef _MD_CLIENT_H_
#define _MD_CLIENT_H_

#include "list.h"
#include <stdint.h>
#include "kapi.h"

/**
 initialize server connection
 */
int server_create_local(char *path);

int server_create_lustre(char *nid, char *fsname);

void server_destroy(void);


const char *md_stat_name(int md_op);

struct vm_program;
/**
 test client settings
 */
struct md_client {
	/**
	 linkage into list of clients
	 */
	struct list_head	mdc_link;
	/**
	 client name, must be unique.
	*/
	char			*mdc_name;
	/**
	 client id, must be unique
	*/
	long			mdc_id;
	/**
	 connection info
	 */
	struct simul_mnt	*mdc_mds;
	/**
	 program run on that client
	 */
	struct vm_program	*mdc_prg;
	/**
	 status of program execution
	 */
	uint32_t		mdc_rc;
	/**
	 last operation executed
	 */
	uint32_t		mdc_op;
	/**
	 time of total execution for given program
	*/
	uint64_t		mdc_time;
	/**
	*/
	uint32_t		mdc_stats_num;
	struct simul_stat_op	*mdc_stats;
};

extern struct list_head clients;

/**
 create new client
 */
int client_create(char *name, char *program);

int clients_get_stats(void);

void clients_destory(void);

#endif
