#include "stdio.h"
#include "stdlib.h"

#include "debug.h"
#include "kernel.h"

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
}