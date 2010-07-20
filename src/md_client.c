#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "md_client.h"
#include "kernel.h"
#include "vm_compile.h"

static char *server_uid;
static char *server_nid;

int server_create(char *name, char *nid)
{
	server_uid = strdup(name);
	server_nid = strdup(nid);

	return 0;
}

int client_create(char *name, char *program)
{
	struct vm_program *prg;

	prg = vm_program_find(program);
	if (prg == NULL)
		return -ESRCH;

	return simul_api_cli_create(name, server_nid, 
				    prg->vmp_data, prg->vmp_enc_idx);
}
