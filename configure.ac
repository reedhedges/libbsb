dnl $Id: configure.ac,v 1.25 2007/02/18 07:35:04 mikrom Exp $
dnl
dnl Process this file with autoconf to produce a configure script.
AC_INIT(bsb2tif.c)
AM_INIT_AUTOMAKE(libbsb, 0.0.8)

dnl AC_PROG_CC finds CC and defaults CFLAGS (usually -g -O2)
orig_cflags=$CFLAGS
AC_PROG_CC
AC_PROG_CXX

dnl Default CFLAGS for developer's convenience and avoid ranlib with MSVC
use_msvc=no
case "$CC" in
	cl*|CL*)
		# -Od turn off optimisation, -Zi turn on debugging info
		CFLAGS="-nologo -Od -W3 -Zi"
		# Use a no-op command for ranlib
		RANLIB="true"
		use_msvc=yes
		;;
	*)
		# override AC_PROG_CC default CFLAGS except when user specified CFLAGS
		if test x"$orig_cflags" != x"$CFLAGS" ; then
			CFLAGS="-std=gnu99 -D_GNU_SOURCE -g -O2 -W -Wall -Werror -fPIC"
		fi
		AC_PROG_RANLIB
		;;
esac
AM_CONDITIONAL(USE_MSVC, test x"$use_msvc" = "xyes")


dnl disable building programs, library only
AC_ARG_ENABLE(programs, 
  [AS_HELP_STRING([--disable-programs], [Only build libbsb library. Default (or if --enable-programs given) is to build libbsb library and example/utility conversion programs.])],
  #[echo arg given enable_programs=$enable_programs enableval=$enableval], # AC_ARG_ENABLE will set enable_programs to yes for --enable or no for --disable
  [],
  [enable_programs=yes]
)

AM_CONDITIONAL(ENABLE_PROGRAMS, test x"$enable_programs" = "xyes")

if test x"$enable_programs" = "xyes"
then
  AC_MSG_NOTICE([Example/utility programs enabled.])

  dnl If libtiff found, build TIFF programs: tif2bsb, bsb2tif
  have_libtiff=no
  AC_CHECK_HEADER([tiff.h],
    [AC_CHECK_LIB(tiff, TIFFOpen, [ have_libtiff=yes ],
      [], [-lm])]
    )
  AM_CONDITIONAL(HAVE_LIBTIFF, test x"$have_libtiff" = "xyes")
  if test x"$have_libtiff" = "xno" ; then
    AC_MSG_WARN([*** TIFF conversion tools will not be built ***])
  fi

  dnl If libpng found, build PNG programs: tif2png
  have_libpng=no
  AC_CHECK_HEADER([png.h],
    [AC_CHECK_LIB(png, png_init_io, [ have_libpng=yes ],
      [], [-lz -lm])]
    )
  AM_CONDITIONAL(HAVE_LIBPNG, test x"$have_libpng" = "xyes")
  if test x"$have_libpng" = "xno" ; then
    AC_MSG_WARN([*** PNG conversion tools will not be built ***])
  fi

  have_libzip=no
  AC_CHECK_HEADER([zip.h],
    [AC_CHECK_LIB(zip, zip_open, [ have_libzip=yes ], [], [])]
  )
  AM_CONDITIONAL(HAVE_LIBZIP, test x"$have_libzip" = "xyes")
  if test x"$have_libzip" = "xno"; then
    AC_MSG_WARN([*** ZIP conversion tools will not be built ***])
  fi

  have_qt=no
  have_libqt=no
  AC_ARG_WITH([qt], [use qt4 installed at the given location to build bsbview, if possible. default is to use environment QTDIR variable], qtdir=$withval, qtdir=${QTDIR})
  if test x"$qtdir" = "xno"
  then
    AC_MSG_NOTICE([--without-qt given, skipping Qt checks, bsbview will not be built.])
  else
    AC_MSG_NOTICE([Checking for Qt4 in $qtdir...])
    AC_PATH_X
    if test x"$qtdir" = "x" ; then
            dnl Try to locate Qt4 in well-known places
            AC_CHECK_FILE([/usr/share/qt4],
                          [qtdir=/usr/share/qt4],
                          [qtdir=/usr/lib/qt4])
            export QTDIR=$qtdir
      AC_MSG_WARN([QTDIR not set, assuming $qtdir but build will fail if QTDIR env var not set])
    fi
    qt_includes=$qtdir/include
    export CPPFLAGS="$CPPFLAGS -I$qt_includes"
    AC_CHECK_HEADER([QtCore/qglobal.h], [ have_libqt=yes ] )
  fi

  AM_CONDITIONAL(HAVE_LIBQT, test x"$have_libqt" = "xyes")
  if test x"$have_libqt" = "xyes" ; then
     AC_MSG_NOTICE([Qt4 seems OK. Including bsbview in the build!])
  else
     AC_MSG_NOTICE([bsbview will not be built.])
  dnl   AC_MSG_WARN([*** bsbview will not be built (no QT or QTDIR wrong) ***])
  fi

else

  AC_MSG_NOTICE([Example/utility programs disabled. Only libbsb library will be built.])

  AM_CONDITIONAL(HAVE_LIBTIFF, false)
  AM_CONDITIONAL(HAVE_LIBPNG, false)
  AM_CONDITIONAL(HAVE_LIBZIP, false)
  AM_CONDITIONAL(HAVE_LIBQT, false)
  
fi

dnl Test for valgrind and test which args are needed for memcheck.
dnl Using -q --leak-check on valgrind pre-2.2 gives stderr output when
dnl no leaks are found so only use --leak-check with recent valgrind.
AC_CHECK_PROGS(VALGRIND, valgrind)
if test x"$VALGRIND" != "x" ; then
	if $VALGRIND -q --tool=memcheck true 2>/dev/null; then
		VALGRIND="valgrind --tool=memcheck --leak-check=yes"
	fi
fi

dnl Initialize the test suite
dnl (the use of '.' is needed to get top-level in the PATH under FreeBSD)
AC_CONFIG_TESTDIR([tests], [tests:.])
AC_CONFIG_FILES([tests/Makefile])
AC_CONFIG_FILES([tests/at_wrap], [chmod +x tests/at_wrap])
AM_MISSING_PROG([AUTOM4TE], [autom4te])

AC_OUTPUT(Makefile)
