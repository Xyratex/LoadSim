#ifndef _MDSIM_KAPI_H_
#define _MDSIM_KAPI_H_

/**
 kernel to user application interface API

 kernel API present a operation over character device, 
 exported from a kernel module.
 
 typical usage of simulator is
 1) open a device node. kernel is initialize internal states and stay ready to create
    environments and virtual machines.
 2) upload programs. each client spawn execute thread and wait until all clients
    initialized
 3) call run command. all threads wake-up and run own program.
 4) userland application stay in waiting test completed 
 5) get results and statistic after execution
 6) close an opened device node. 
*/

#define SIMUL_DEV_NAME	"c2_ksim"

/**
 kernel module implement a lots operations, which available via IOCTL call.
*/
enum simul_ops {
	/**
	 create a metadata client instance and upload program into kernel
	*/
	SIM_IOW_MDCLIENT	= 0x8800,
	/**
	 run uploaded programs
	 */
	SIM_IOW_RUN,
	/**
	 get results after all tests a finished.
	 @see struct simul_res
	 */
	SIM_IOW_RESULTS,
};

#ifndef __user
#define __user
#endif

/**
 parameters to a create lustre metadata client
 */
struct simul_ioctl_cli {
	char __user	*sic_name;
	long		 sic_id;
	char __user	*sic_dst_fs;
	char __user	*sic_dst_nid;
	int		sic_programsz;
	char __user	*sic_program;
	int		sic_regs;
};

/**
 array to return from operation SIM_IOW_RESULTS
 */
struct simul_stat_op {
	int64_t sso_op_id;
	int64_t sso_min_time;
	int64_t sso_max_time;
	int64_t sso_avg_time;
};

struct simul_ioctl_res {
	uint32_t		ss_cli;
	uint32_t		*ss_res;
	uint32_t		*ss_ip;
	uint64_t		*ss_time; /** < time to execute program in ms */
	struct simul_stat_op __user *ss_stats; /** < fill by kernel */
};
#endif
