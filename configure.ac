AC_PREREQ([2.59])
AC_INIT([loadsim], [0.1], [lustre@clusterstor.com])
AC_CONFIG_HEADER([loadsim_c.h])

AH_TEMPLATE([PACKAGE], [Package name.])
AH_TEMPLATE([VERSION], [Version of the package.]) 
AH_TEMPLATE([DEBUG], [Enable debug prints])

PACKAGE=loadsim
VERSION=0.1
SYMVERFILE=Module.symvers

AM_INIT_AUTOMAKE([$PACKAGE], [$VERSION]) 

AC_DEFUN([COLIBRI_LINUX_INIT_CONFIG], [
rm -fr config > /dev/null 2>&1
mkdir config
cat >config/Makefile <<_ACEOF
obj-m := conftest.o

modules:
	\$(MAKE) -C \$(LINUX_OBJ) M=\`pwd\` modules
_ACEOF
])

# New macros for checking various things
AC_DEFUN([COLIBRI_LINUX_SYMVER],[
        AC_MSG_CHECKING([for symbols file])
        if grep -q Modules.symvers $LINUX/scripts/Makefile.modpost ; then
                SYMVERFILE=Modules.symvers
	fi
	AC_MSG_RESULT([$LINUX/$SYMVERFILE])
])

# New macros for checking various things
AC_DEFUN([COLIBRI_LINUX_CHECK_SYMBOL],[
        AC_MSG_CHECKING([for symbol $1 exported])
        grep -q -E '[[[:space:]]]$1[[[:space:]]]' $LINUX/$SYMVERFILE 2>/dev/null
        rc=$?
        if test $rc -eq 0; then
                AC_MSG_RESULT([yes])
                $2
        else
                AC_MSG_RESULT([no])
                $3
        fi
])

AC_DEFUN([COLIBRI_LINUX_CONFTEST],
[cat >conftest.c <<_ACEOF
$1
_ACEOF
])

AC_DEFUN([COLIBRI_C_PROGRAM],
[$1
int
main (void)
{
dnl Do *not* indent the following line: there may be CPP directives.
dnl Don't move the `;' right after for the same reason.
$2
  ;
  return 0;
}])

AC_DEFUN([COLIBRI_LINUX_COMPILE_IFELSE],
[m4_ifvaln([$1], [COLIBRI_LINUX_CONFTEST([$1])])
rm -f config/conftest.o config/conftest.mod.c config/conftest.ko
AS_IF([AC_TRY_COMMAND([cp conftest.c config && cd config && make -d $2 ${LD:+"LD=$LD"} CC="$CC" LINUX_OBJ=$LINUX_OBJ]) >/dev/null && AC_TRY_COMMAND([$3])],
	[$4],
	[_AC_MSG_LOG_CONFTEST
m4_ifvaln([$5],[$5])])
rm -f config/conftest.o config/conftest.mod.c config/conftest.mod.o config/conftest.ko m4_ifval([$1], [config/conftest.c conftest.c])[]
])

AC_DEFUN([COLIBRI_LINUX_TRY_COMPILE],[
        COLIBRI_LINUX_COMPILE_IFELSE(
                [AC_LANG_SOURCE([COLIBRI_C_PROGRAM([[$1]], [[$2]])])],
                [modules],
                [test -s config/conftest.o],
                [$3], [$4])
])

AC_DEFUN([SIM_FS_PATH], [
AC_MSG_CHECKING([fs_struct use path structure])
COLIBRI_LINUX_TRY_COMPILE([
        #include <asm/atomic.h>
        #include <linux/spinlock.h>
        #include <linux/fs_struct.h>
],[
        struct path path;
        struct fs_struct fs;

        fs.pwd = path;
], [
        AC_MSG_RESULT([yes])
        AC_DEFINE(HAVE_FS_PATH, ,
                [fs_struct use path structure])
],[
        AC_MSG_RESULT([no])
])
])

# in rhel6 vfs_symlink return to use 3 args mode
AC_DEFUN([SIM_VFS_SYMLINK_4],
[AC_MSG_CHECKING([if vfs_symlink have 4 args])
COLIBRI_LINUX_TRY_COMPILE([
	#include <linux/fs.h>
],[
	struct inode *dir;
	struct dentry *dentry;
	const char *oldname = NULL;
	int mode = 0;

	vfs_symlink(dir, dentry, oldname, mode);
],[
        AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_VFS_SYMLINK_4, ,
                  [vfs_symlink have 4 args])
],[
        AC_MSG_RESULT(no)
])
])

AC_DEFUN([SIM_VFS_ALTROOT],
[AC_MSG_CHECKING([kernel has support for alroot])
COLIBRI_LINUX_TRY_COMPILE([
	#include <linux/fs_struct.h>
],[
	struct fs_struct *old;
	old->altroot = NULL;
],[
        AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_VFS_ALTROOT, ,
                  [vfs has altroot])
],[
        AC_MSG_RESULT(no)
])
])

# wrappers for task's credentials are in sles11
#
AC_DEFUN([SIM_CRED_WRAPPERS], [
AC_MSG_CHECKING([if kernel has wrappers for task's credentials])
COLIBRI_LINUX_TRY_COMPILE([
       #include <linux/sched.h>
],[
       uid_t uid;

       uid = current_uid();
],[
       AC_MSG_RESULT([yes])
       AC_DEFINE(HAVE_CRED_WRAPPERS, 1, [task's cred wrappers found])
],[
       AC_MSG_RESULT([no])
])
])

#
# struct cred was introduced in 2.6.29 to streamline credentials in task struct
#
AC_DEFUN([SIM_STRUCT_CRED_IN_TASK], [
AC_MSG_CHECKING([if kernel has struct cred])
COLIBRI_LINUX_TRY_COMPILE([
       #include <linux/sched.h>
],[
       struct task_struct *tsk = NULL;
       tsk->real_cred = NULL;
],[
       AC_MSG_RESULT([yes])
       AC_DEFINE(HAVE_STRUCT_CRED, 1, [struct cred found])
],[
       AC_MSG_RESULT([no])
])
])


AC_DEFUN([SIM_KERNEL], [
        AC_MSG_CHECKING([for linux sources dir])
        AC_ARG_WITH([linux],
                [AC_HELP_STRING([--with-linux=path], [set path to linux sources])],
                [LINUX=$with_linux],
                [LINUX=/lib/modules/`uname -r`/build]
        )
        AC_MSG_RESULT([$LINUX])
        AC_SUBST([LINUX])

        AC_MSG_CHECKING([for linux objects dir])
        AC_ARG_WITH([linux-obj],
                [AC_HELP_STRING([--with-linux-obj=path], [set path to linux objects dir])],
                [LINUX_OBJ=$with_linux_obj],
                [LINUX_OBJ=$LINUX]
        )
        AC_MSG_RESULT([$LINUX_OBJ])
        AC_SUBST([LINUX_OBJ])

	KMOD_DIR="/lib/modules/`uname -r`"
	AC_SUBST([KMOD_DIR]) 

#        AC_MSG_CHECKING([for lustre source dir])
#        AC_ARG_WITH([lustre-src],
#                [AC_HELP_STRING([--with-lustre-src=path], [set path to linux objects dir])],
#                [LUSTRE_SRC=$with_lustre_src]
#        )

        COLIBRI_LINUX_INIT_CONFIG
        COLIBRI_LINUX_SYMVER
        COLIBRI_LINUX_CHECK_SYMBOL([set_normalized_timespec], 
                                   [AC_DEFINE([HAVE_NORM_TIMESPEC], [], [set_normalized_timespec exist on kernel])], [])
        SIM_FS_PATH
        SIM_VFS_SYMLINK_4
        COLIBRI_LINUX_CHECK_SYMBOL([inode_permission], 
                                   [AC_DEFINE([HAVE_INODE_PERM], [], [inode_permission exist on kernel])], [])
        SIM_VFS_ALTROOT
        SIM_STRUCT_CRED_IN_TASK
        SIM_CRED_WRAPPERS

])

AC_DEFUN([SIM_USERLAND], [
	AC_PROG_LEX
	AC_PROG_YACC
])

AC_PROG_CC

AC_ARG_ENABLE([debug],
        [AC_HELP_STRING([--enable-debug], [enable debug information and logs])], 
        [enable_debug=yes]
)
KCC=""

if test "x$enable_debug" = xyes; then
	KCC="$KCC -g"
	AC_DEFINE(DEBUG, [1])
else
	AC_DEFINE(DEBUG, [0])
fi
AC_SUBST(KCC)


SIM_KERNEL

SIM_USERLAND

AC_OUTPUT([
Makefile
src/Makefile
ksrc/Makefile
include/Makefile
tests/Makefile
])
