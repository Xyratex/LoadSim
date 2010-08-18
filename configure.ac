AC_PREREQ([2.59])
AC_INIT([loadsim], [0.1], [lustre@clusterstor.com])
AC_CONFIG_HEADER([loadsim_c.h])
# AC_CONFIG_MACRO_DIR([m4])


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
])

AC_DEFUN([SIM_USERLAND], [
	AC_PROG_LEX
	AC_PROG_YACC
])

AM_INIT_AUTOMAKE

AH_TEMPLATE([LINUX], [path to linux sources])
AH_TEMPLATE([LINUX_OBJ], [path to linux sources])
AH_TEMPLATE([DEBUG], [enable debug])

AC_PROG_CC

AC_ARG_ENABLE([debug],
        [AC_HELP_STRING([--enable-debug], [enable debug information and logs])], 
        [enable_debug=yes]
)
KCC=""
if test xenable_debug == xyes; then
	KCC="$KCC -g"
	AC_DEFINE(DEBUG)
fi
AC_SUBST(DEBUG)
AC_SUBST(KCC)


SIM_KERNEL
SIM_USERLAND

AC_OUTPUT([
Makefile
src/Makefile
ksrc/Makefile
include/Makefile
])
