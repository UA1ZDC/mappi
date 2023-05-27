AC_INIT([Google C++ Testing Framework],
        [1.7.0],
        [googletestframework@googlegroups.com],
        [gtest])

# Provide various options to initialize the Autoconf and configure processes.
AC_PREREQ([2.59])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([build-aux/config.h])
AC_CONFIG_FILES([Makefile])

# Initialize Automake with various options. We require at least v1.9, prevent
# pedantic complaints about package files, and enable various distribution
# targets.
AM_INIT_AUTOMAKE([1.9 dist-bzip2 dist-zip foreign subdir-objects])

# Check for programs used in building Google Test.
AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
AC_PROG_LIBTOOL

PTHREAD_LIBS=""
AC_CHECK_LIB(pthread, pthread_create, [
    PTHREAD_LIBS="-lpthread"
    have_pthreads="yes"
])

AM_CONDITIONAL([HAVE_PTHREADS],[test "x$have_pthreads" = "xyes"])
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

# Output the generated files. No further autoconf macros may be used.
AC_OUTPUT
