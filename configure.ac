#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libtw], [0.0.0], [van.fisted@googlemail.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/testlibtw.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
LT_PREREQ([2.4])
AM_PROG_AR
LT_INIT

#AC_LANG([C++])
AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([ext], [mandatory])
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_CHECK_LIB([srsbsns], [addr_mksocket], [], [AC_MSG_ERROR([libsrsbsns not installed?])])
AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR([no pthreads?])])

AC_CHECK_HEADER_STDBOOL
AC_CHECK_HEADERS([arpa/inet.h sys/socket.h sys/time.h])

AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

AC_CHECK_FUNCS([gettimeofday memset select strdup strtol])

AC_CONFIG_FILES([Makefile
		 include/Makefile
		 include/libtw/Makefile
		 libtw/Makefile
                 src/Makefile])
AC_OUTPUT
echo \
"-------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION}

 Prefix: '${prefix}'.
 C Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
 C++ Compiler: '${CXX} ${CXXFLAGS} ${CPPFLAGS}'
 Libraries: '${LIBS}'

--------------------------------------------------"
