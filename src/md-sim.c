#include "stdio.h"
#include "stdlib.h"

#include "debug.h"
#include "list.h"
#include "vm_compile.h"

int main(int argc, char *argv[])
{
	struct vm_program *prog;
	int rc;

	rc = yyparse();
	if (rc) {
		err_print("can't parse config\n");
		return 1;
	}

	/* setup clients */
}