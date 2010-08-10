#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "list.h"
#include "md_client.h"
#include "kernel.h"
#include "vm_compile.h"
#include "kapi.h" /* XXXX */

static struct server_link sl;

int server_create(char *name, char *fs, char *nid)
{
	sl.sl_name = strdup(name);
	sl.sl_fs = strdup(fs);
	sl.sl_nid = strdup(nid);

	return 0;
}

LIST_HEAD(clients);
static long cliidx;

int client_create(char *name, char *program)
{
	struct vm_program *prg;
	struct md_client *cli;
	int rc;

	prg = vm_program_find(program);
	if (prg == NULL)
		return -ESRCH;

	if (sl.sl_name == NULL)
		return -ENODATA;

	cli = malloc(sizeof *cli);
	if (cli == NULL)
		return -ENOMEM;

	cli->mdc_name = strdup(name);
	if (cli->mdc_name == NULL) {
		free(cli);
		return -ENOMEM;
	}
	cli->mdc_id = cliidx ++;
	cli->mdc_prg = prg;
	cli->mdc_mds = &sl;
	cli->mdc_rc = -ENODATA;
	cli->mdc_op = -1;

	rc = simul_api_cli_create(cli->mdc_name, cli->mdc_id, cli->mdc_mds,
				  prg->vmp_data, prg->vmp_enc_idx);
	if (rc == 0) {
		list_add(&cli->mdc_link, &clients);
	} else {
		free(cli->mdc_name);
		free(cli);
	}

	return rc;
}

static unsigned int clients_get_count()
{
	unsigned int i = 0;
	struct list_head *tmp;

	list_for_each(tmp, &clients) {
		i ++;
	}

	return i;
}

static struct md_client *clients_find_by_id(long id)
{
	struct md_client *cli;

	list_for_each_entry(cli, &clients, mdc_link)
		if (cli->mdc_id == id)
			return cli;

	return NULL;
}

int clients_get_stats(void)
{
	int clicnt = clients_get_count();
	struct simul_res *data;
	int rc = 0;

	if (clicnt == 0)
		return -ENODATA;

	data = malloc(sizeof (*data) * clicnt);
	if (data == NULL)
		return -ENOMEM;

	rc = simul_api_get_results(data);
	if (rc == 0) {
		int i;
		struct md_client *cli;

		for (i = 0; i < clicnt; i++) {
			cli = clients_find_by_id(data[i].sr_cli);
			if (cli != NULL) {
				cli->mdc_rc = data[i].sr_res;
				cli->mdc_op = data[i].sr_ip;
			} else
				rc = -ESRCH;
		}
	}

	free(data);
	return rc;
}
