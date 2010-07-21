#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "kapi.h"

static int api_fd = -1;

int simul_api_open()
{
	api_fd = open("/dev/"SIMUL_DEV_NAME, O_RDONLY);
	if (api_fd == -1)
		return -errno;

	return 0;
}

int simul_api_cli_create(char *name, char *dstnid, void *data, int size)
{
	struct simul_ioctl_cli _data;

	if (api_fd == -1)
		return -ENOSYS;

	_data.sic_name = name;
	_data.sic_dst_nid = dstnid;
	_data.sic_program = data;
	_data.sic_programsz = size;

	return ioctl(api_fd, SIM_IOW_MDCLIENT, &_data);
}

int simul_api_run()
{
	return ioctl(api_fd, SIM_IOW_RUN, NULL);
}

int simul_api_wait_finished()
{
	/* XXX pool*/
}

int simul_api_get_results(long *res)
{
	if (api_fd == -1)
		return -ENOSYS;

	return ioctl(api_fd, SIM_IOW_RESULTS, res);
}

int simul_api_close()
{
	close(api_fd);
	api_fd = -1;
}