#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "md_client.h"
#include "kernel.h"
#include "vm_compile.h"

static struct server_link sl;

int server_create(char *name, char *fs, char *nid)
{
	sl.sl_name = strdup(name);
	sl.sl_fs = strdup(fs);
	sl.sl_nid = strdup(nid);

	return 0;
}

int client_create(char *name, char *program)
{
	struct vm_program *prg;

	prg = vm_program_find(program);
	if (prg == NULL)
		return -ESRCH;

	return simul_api_cli_create(name, &sl,
				    prg->vmp_data, prg->vmp_enc_idx);
}
