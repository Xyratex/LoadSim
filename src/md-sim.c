#include "stdio.h"
#include "stdlib.h"

#include "md_client.h"
#include "vm_compile.h"
#include "debug.h"
#include "kernel.h"

#include "xml/ezxml.h"

#if 0
void client_res(struct md_client *cli)
{
	int i = 0;
	printf("cli %s : last op %d total ops %d total time %lu(ms) rc %d\n",
	       cli->mdc_name, cli->mdc_op, cli->mdc_prg->vmp_enc_idx, 
	       cli->mdc_time,
	       cli->mdc_rc);

	for(i=0; i<cli->mdc_stats_num; i++) {
		printf("\t%s : %u/%u/%u\n", 
			md_stat_name(cli->mdc_stats[i].sso_op_id),
			cli->mdc_stats[i].sso_min_time,
			cli->mdc_stats[i].sso_max_time,
			cli->mdc_stats[i].sso_avg_time);
	}
}
#endif

void xml_add_num(ezxml_t xml, const char *name, unsigned long val)
{
	char num[20]; /* MAX_LONG + \0*/

	snprintf(num, sizeof(num), "%lu", val);
	ezxml_set_attr_d(xml, name, num);
}

void xml_vm_stat(ezxml_t xml, struct md_client *cli)
{
	xml_add_num(xml, "last_op", cli->mdc_op);
	xml_add_num(xml, "total_ops", cli->mdc_prg->vmp_enc_idx);
	xml_add_num(xml, "total_time", cli->mdc_time);
	xml_add_num(xml, "status", cli->mdc_rc);
}

void xml_add_one_stat(ezxml_t xml, struct simul_stat_op *stat)
{
	ezxml_set_attr_d(xml, "name", md_stat_name(stat->sso_op_id));
	xml_add_num(xml, "min_time", stat->sso_min_time);
	xml_add_num(xml, "max_time", stat->sso_max_time);
	xml_add_num(xml, "avg_time", stat->sso_avg_time);
}

void client_res(ezxml_t root, struct md_client *cli)
{
	int i;
	char num[20]; /* MAX_LONG + \0*/
	ezxml_t vm;
	ezxml_t stats;

	ezxml_set_attr_d(root, "name", cli->mdc_name);
	vm = ezxml_add_child(root, "VM", 1);
	if (vm) {
		xml_vm_stat(vm, cli);
	}

	stats = ezxml_add_child(root, "stats", 2);
	if (stats) {
		ezxml_t st;

		xml_add_num(stats, "count", cli->mdc_stats_num);
		for(i = 0; i < cli->mdc_stats_num; i++) {
			if (cli->mdc_stats[i].sso_avg_time == 0)
				continue;
			st = ezxml_add_child(stats, "stat", i);
			if (st) {
				xml_add_one_stat(st, &cli->mdc_stats[i]);
			} else {
				fprintf(stderr, "don't able add stat to xml");
			}
		}
	}
}


void client_stats(void)
{
	ezxml_t root;
	ezxml_t xml;
	struct md_client *cli;
	char *s;

	root = ezxml_new("mdsimstats");
	xml_add_num(root, "numclients", 0);

	list_for_each_entry(cli, &clients, mdc_link) {
		xml = ezxml_add_child(root, "client", 1);
		if (xml) {
			client_res(xml, cli);
		} else {
			fprintf(stderr, "can't create a new client\n");
		}
	}
	s = ezxml_toxml(root);
	fprintf(stdout, "%s\n", s);
	free(s);
}

int main(int argc, char *argv[])
{
	int rc;

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

	simul_api_close();

	client_stats();
}
