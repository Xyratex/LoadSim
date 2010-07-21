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
	SIM_IOW_MDCLIENT,
	/**
	 run uploaded programs
	 */
	SIM_IOW_RUN,
	/**
	 get results after all tests a finished
	 */
	SIM_IOW_RESULTS,
	/**
	 get statistic for each clients
	 */
	SIM_IOW_STATS,
};

#ifndef __user
#define __user
#endif

/**
 parameters to a create lustre metadata client
 */
struct simul_ioctl_cli {
	char __user	*sic_name;
	char __user	*sic_dst_uuid;
	char __user	*sic_dst_nid;
	int		sic_programsz;
	char __user	*sic_program;
};

#endif
