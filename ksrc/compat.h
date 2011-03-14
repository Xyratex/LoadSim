#ifndef _SIMUL_COMPAT_H_
#define _SIMUL_COMPAT_H_

#include "loadsim_c.h"

#ifdef HAVE_VFS_SYMLINK_4
#define sim_vfs_symlink(a,b,c,d) vfs_symlink((a),(b),(c),(d))
#else
#define sim_vfs_symlink(a,b,c,d) vfs_symlink((a),(b),(c))
#endif


#ifdef HAVE_FS_PATH
#define sim_nd_dentry(nd)    ((nd).path.dentry)
#define sim_nd_mnt(nd)       ((nd).path.mnt)

#define sim_fs_pwd(fs)       ((fs)->pwd.dentry)
#define sim_fs_pwdmnt(fs)    ((fs)->pwd.mnt)
#define sim_fs_root(fs)      ((fs)->root.dentry)
#define sim_fs_rootmnt(fs)   ((fs)->root.mnt)


#define sim_path_put(nd)     path_put(&(nd)->path)
#else
#define sim_nd_dentry(nd)    ((nd)->dentry)
#define sim_nd_mnt(nd)       ((nd)->mnt)

#define sim_fs_pwd(fs)       ((fs)->pwd)
#define sim_fs_pwdmnt(fs)    ((fs)->pwdmnt)
#define sim_fs_pwd(fs)       ((fs)->root)
#define sim_fs_pwdmnt(fs)    ((fs)->rootmnt)

#define sim_path_put(nd)     path_release(nd)
#endif

#ifdef HAVE_INODE_PERM
#define sim_exec_permission(nd) inode_permission(sim_nd_dentry(nd)->d_inode, MAY_EXEC | MAY_ACCESS)
#define sim_write_permission(nd) inode_permission(sim_nd_dentry(nd)->d_inode, MAY_WRITE)
#else
#define sim_exec_permission(nd) vfs_permission(&(nd), MAY_EXEC)
#define sim_write_permission(nd) vfs_permission(&(nd), MAY_WRITE)
#endif 

#ifndef HAVE_VFS_ALTROOT
#define LOOKUP_NOALT   0
#endif

/** 
 XXX direct copy from lustre
*/
#ifndef HAVE_STRUCT_CRED

#define current_cred() (current)

#define current_cred_xxx(xxx)                   \
({                                              \
        current->xxx;                     \
})

#ifndef HAVE_CRED_WRAPPERS

#define current_uid()           (current_cred_xxx(uid))
#define current_gid()           (current_cred_xxx(gid))
#define current_euid()          (current_cred_xxx(euid))
#define current_egid()          (current_cred_xxx(egid))
#define current_suid()          (current_cred_xxx(suid))
#define current_sgid()          (current_cred_xxx(sgid))
#define current_fsuid()         (current_cred_xxx(fsuid))
#define current_fsgid()         (current_cred_xxx(fsgid))
#define current_cap()           (current_cred_xxx(cap_effective))

#endif /* HAVE_LINUX_CRED_H */

#define current_user()          (current_cred_xxx(user))
#define current_user_ns()       (current_cred_xxx(user)->user_ns)
#define current_security()      (current_cred_xxx(security))

#define cred task_struct

#define prepare_creds() (current)
#define commit_creds(a)

#endif /* HAVE_STRUCT_CRED */


#endif