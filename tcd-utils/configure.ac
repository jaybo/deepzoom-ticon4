# $Id: configure.ac 8292 2024-02-22 20:33:25Z flaterco $

# FIXME: Update version info on each release
AC_INIT([tcd-utils], [20240222], [dave@flaterco.com])

AM_INIT_AUTOMAKE([-Wall -Werror])

AC_LANG([C])
AC_PROG_CC

LIBS="$LIBS -lm"

# libtcd 2.2 or compatible newer version is required.
AC_CHECK_HEADER([tcd.h], [],
                [AC_MSG_ERROR([cannot find tcd.h; try setting CPPFLAGS.])])
AC_CHECK_LIB([tcd], [get_tide_db_header], [],
             [AC_MSG_ERROR([cannot find libtcd; try setting LDFLAGS.])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
