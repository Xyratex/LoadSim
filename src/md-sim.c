#include "stdio.h"
#include "stdlib.h"

#include "md_client.h"
#include "vm_compile.h"
#include "debug.h"
#include "kernel.h"

void client_res(struct md_client *cli)
{
	int i = 0;
	printf("cli %s : last op %d total %d rc %d\n",
	       cli->mdc_name, cli->mdc_op, cli->mdc_prg->vmp_enc_idx, cli->mdc_rc);

	for(i=0; i<cli->mdc_stats_num; i++) {
		printf("\t%s : %u/%u/%u\n", 
			md_stat_name(cli->mdc_stats[i].sso_op_id),
			cli->mdc_stats[i].sso_min_time,
			cli->mdc_stats[i].sso_max_time,
			cli->mdc_stats[i].sso_avg_time);
	}
}

int main(int argc, char *argv[])
{
	int rc;
	struct md_client *cli;

	rc = simul_api_open();
	if (rc < 0) {
		err_print("can't open kernel interface\n");
		return 1;
	}

	rc = yyparse();
	if (rc) {
		err_print("can't parse config\n");
		return 1;
	}

	simul_api_run();

	simul_api_wait_finished();

        clients_get_stats();
	list_for_each_entry(cli, &clients, mdc_link) {
		client_res(cli);
	}

	simul_api_close();
}
