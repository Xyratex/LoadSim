#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include "md_client.h"
#include "kapi.h"
#include "debug.h"

static int api_fd = -1;

int simul_api_open()
{
	api_fd = open("/dev/"SIMUL_DEV_NAME, O_RDONLY);
	if (api_fd == -1)
		return -errno;

	return 0;
}

int simul_api_cli_create(char *cliname, long cliid, struct server_link *sl,
			 void *data, int size)
{
	struct simul_ioctl_cli _data;

	if (api_fd == -1)
		return -ENOSYS;

	_data.sic_name = cliname;
	_data.sic_id = cliid;
	_data.sic_dst_fs = sl->sl_fs;
	_data.sic_dst_nid = sl->sl_nid;
	_data.sic_program = data;
	_data.sic_programsz = size;
	DPRINT("program %p\n", data);

	return ioctl(api_fd, SIM_IOW_MDCLIENT, &_data);
}

int simul_api_run()
{
	return ioctl(api_fd, SIM_IOW_RUN, NULL);
}

int simul_api_wait_finished()
{
	struct pollfd _poll;
	int rc;

	if (api_fd == -1)
		return -ENOSYS;

	_poll.fd = api_fd;
	_poll.events = POLLIN;

	while (1) {
		rc = poll(&_poll, 1, -1);
		DPRINT("rc %d - %x\n", rc, _poll.revents);
		if (rc < 0) {
			rc = -errno;
			break;
		}
		if ((rc > 0) && (_poll.revents & POLLIN)) {
			rc = 0;
			break;
		}
	}

	return rc;
}

int simul_api_get_results(uint32_t id, uint32_t *res, uint32_t *ip,
                          uint64_t *time,
		          struct simul_stat_op *data)
{
	struct simul_ioctl_res _res;
	if (api_fd == -1)
		return -ENOSYS;

	_res.ss_cli = id;
	_res.ss_res = res;
	_res.ss_time = time;
	_res.ss_ip = ip;
	_res.ss_stats = data;

	return ioctl(api_fd, SIM_IOW_RESULTS, (long)&_res);
}

int simul_api_close()
{
	close(api_fd);
	api_fd = -1;
}

