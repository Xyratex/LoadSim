#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "debug.h"
#include "list.h"
#include "md_client.h"
#include "kernel.h"
#include "vm_defs.h"
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
static long cliidx = 0;

void client_destroy(struct md_client *cli)
{
	if (cli == NULL)
		return;

	if (cli->mdc_name != NULL)
		free(cli->mdc_name);

	if (cli->mdc_stats != NULL)
		free(cli->mdc_stats);

	free(cli);
}

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
	memset(cli, 0, sizeof *cli);

	cli->mdc_name = strdup(name);
	if (cli->mdc_name == NULL) {
		client_destroy(cli);
		return -ENOMEM;
	}
	cli->mdc_stats = malloc((VM_MD_CALL_MAX - VM_MD_CALL_MAX) * 
				sizeof(*cli->mdc_stats));
	if (cli->mdc_stats == NULL) {
		client_destroy(cli);
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
		client_destroy(cli);
	}

	return rc;

}

/* [start'-'end] */
int get_range(char *prefix, int *start, int *end)
{
	char *r_start;
	char *r_end;
	char *err;
	long st;
	long en;

	r_start = strchr(prefix, '[');
	if (r_start == NULL) {
		*start = 0;
		*end = 0;
		return 0;
	}
	*r_start = '\0'; r_start++;
	st = strtol(r_start, &r_end, 0);
	if (*r_end != '-')
		return -EINVAL;
	r_end ++;

	en = strtol(r_end, &err, 0);
	if (*err != ']')
		return -EINVAL;

	*start = st;
	*end = en;

	return 0;
}

int clients_create(char *prefix, char *prg)
{
	char name[256];
	int start;
	int end;
	int i;
	int rc;

	if (get_range(prefix, &start, &end) < 0)
		return -EINVAL;

	for (i = start; i <= end; i++) {
		snprintf(name, sizeof(name), "%s_%d", prefix, i);
		rc = client_create(&name[0], prg);
		if (rc < 0) {
			err_print("can't create client %s - rc: %d\n", name, rc);
			return rc;
		}
	}

	return 0;
}

static struct md_client *clients_find_by_id(long id)
{
	struct md_client *cli;

	list_for_each_entry(cli, &clients, mdc_link)
		if (cli->mdc_id == id)
			return cli;

	return NULL;
}

static const char *md_names[] = {
	"VM_MD_CALL_CD",
	"VM_MD_CALL_MKDIR",
	"VM_MD_CALL_READDIR",
	"VM_MD_CALL_UNLINK",
	"VM_MD_CALL_OPEN",
	"VM_MD_CALL_CLOSE",
	"VM_MD_CALL_STAT",
	"VM_MD_CALL_CHMOD",
	"VM_MD_CALL_CHOWN",
	"VM_MD_CALL_CHTIME",
	"VM_MD_CALL_TRUNCATE",
	"VM_MD_CALL_SOFTLINK",
	"VM_MD_CALL_HARDLINK",
	"VM_MD_CALL_READLINK",
	"VM_MD_CALL_RENAME",
};
const char *md_stat_name(int md_op)
{
	return md_names[md_op - VM_MD_CALL_CD];
}

int clients_get_stats(void)
{
	struct md_client *cli;
	int rc = 0;

	list_for_each_entry(cli, &clients, mdc_link) {
		cli->mdc_stats_num = VM_MD_CALL_MAX - VM_MD_CALL_CD;
		cli->mdc_stats = malloc(sizeof(*cli->mdc_stats) *
					cli->mdc_stats_num);
		if (cli->mdc_stats == NULL) {
			rc = -ENOMEM;
			continue;
		}
		simul_api_get_results(cli->mdc_id, &cli->mdc_rc, &cli->mdc_op,
				      &cli->mdc_time, cli->mdc_stats);
	}
	return rc;
}
