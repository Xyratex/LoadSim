#ifndef _MD_CLIENT_H_
#define _MD_CLIENT_H_

#include "vm_compile.h"

/**
 connection to meta-data server
*/
struct server_link {
	/**
	 server name to connect
	*/
	char			*sl_name;
	/**
	 NID to connect
	 */
	char			*sl_nid;
};

int server_create(char *, char *);

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
	 connection info
	 */
	struct mds_connect	*mdc_mds;
	/**
	 program run on that client
	 */
	char			*mdc_prg;
};

int client_create(char *name, char *program);

#endif