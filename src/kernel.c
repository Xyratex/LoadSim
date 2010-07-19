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
	struct simul_ioctl_cli data;

	if (api_fd == -1)
		return -ENOSYS;

	return ioctl(api_fd, MDSIM_IOW_CLIENT, &data);
}

int simul_api_wait_finished()
{
	/* XXX pool*/
}

int simul_api_get_results(long *res)
{
	if (api_fd == -1)
		return -ENOSYS;

	return ioctl(api_fd, MDSIM_IOW_RESULTS, &data);
}

int simul_api_close()
{
	close(api_fd);
	api_fd = -1;
}