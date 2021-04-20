AC_PREREQ([2.68])
AC_INIT([liby262], [1.0], [ralf.willenbacher@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([y262api.c])
AC_CONFIG_HEADERS([config.h])
AC_ARG_ENABLE([debug],[AS_HELP_STRING([--enable-debug],[compile and link debug information @<:@default=no@:>@])],[],[makedebug=no])
AS_IF([test "x$makedebug" != xno],[CFLAGS="-O0 -g";CXXFLAGS="-O0 -g"],[])
AS_IF([test "x$CFLAGS" == x],[CFLAGS="-O2 -Wall"],[])
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AM_PROG_AR
AM_PROG_AS
AC_PROG_RANLIB
AC_CHECK_PROG([YASM],[yasm],[yasm],[no])
AS_IF([test "x$YASM" == xno],[AC_MSG_FAILURE([could not find the assembler yasm])],[])
AC_CHECK_LIB([m],[sqrt])
AC_CHECK_LIB([pthread],[pthread_create])
AC_DEFINE([HAVE_LIBPTHREAD], [1], [libpthread available])
AC_CHECK_HEADERS([stdlib.h memory.h string.h])
AC_TYPE_SIZE_T
AC_CHECK_FUNCS([floor pow memset memcpy sqrt])

YASM_I386=no
YASM_X86_64=no
AS_IF([test "x$YASM" != xno],[AS_CASE([$host],
	[*i386*], [YASM_I386=yes],
	[*x86_64*], [YASM_X86_64=yes],
	[]
	)])
AM_CONDITIONAL([WITH_YASM_I386], [test "$YASM_I386" == yes])
AM_CONDITIONAL([WITH_YASM_X86_64], [test "$YASM_X86_64" == yes])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
