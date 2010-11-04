#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libshjpeg], [1.3.1], [dhobsong@igel.co.jp])
AC_SUBST([LIBSHJPEG_VERSION], [1.3.1])

AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/shjpeg_decode.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_WITH([libjpeg-path],
	AS_HELP_STRING([--with-libjpeg-path=<path>],
		[path to libjpeg source files for version to use on target]),
			libjpeg_inc_path=$withval
		fi])

AM_CONDITIONAL(MAKE_JPEG_WRAPPER, test x = x)
AM_CONDITIONAL(CUSTOM_JPEG_LIB, test x$libjpeg_inc_path != x)

PKG_CHECK_MODULES(DIRECTFB, directfb >= 1.4.0, HAVE_DIRECTFB=yes, HAVE_DIRECTFB=no)

AM_CONDITIONAL(MAKE_DIRECTFB_TEST, test x$HAVE_DIRECTFB = xyes)

AC_SUBST([libjpeg_inc_path])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_C_INLINE

# Check for external programs
AC_CHECK_PROG(HAVE_PKG_CONFIG, pkg-config, yes)
AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, true, false)
AM_CONDITIONAL(HAVE_DOXYGEN, $HAVE_DOXYGEN)
if test $HAVE_DOXYGEN = "false"; then
	AC_MSG_WARN([*** doxygen not found, docs will not be built])
fi

# Checks for libraries.
	AC_CHECK_LIB([dl], [dlopen],, [AC_MSG_ERROR([libdl not found!])])
	AC_DEFINE(LIBJPEG_WRAPPER_SUPPORT, 1, [Indicate support of libjpeg wrapper])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h sys/param.h stdint.h string.h sys/ioctl.h unistd.h jpeglib.h malloc.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
#AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([getpagesize memset munmap strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 gen/Makefile
                 doc/Makefile
		 doc/Doxyfile
		 tests/Makefile
		 include/Makefile
		 include/shjpeg/Makefile
		 shjpeg.pc])
AC_OUTPUT
