#include "stdio.h"
#include "stdlib.h"

#include "list.h"
#include "vm_compile.h"

LIST_HEAD(vm_programs);

int main(int argc, char *argv[])
{
	struct vm_program *prog;

	yyparse();
#if 0
	list_for_each_entry(prog, &vm_programs, vmp_link) {
		if (!vm_program_check(prog)) {
			fprintf(stderr, "invalid program - name %s\n",
				prog->vmp_name);
		    exit(1);
		}
	}
#endif
	/* setup clients */
}