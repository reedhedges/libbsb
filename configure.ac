dnl Process this file with autoconf to produce a configure script.

AC_INIT([libbsb], [0.0.9])
AC_CONFIG_SRCDIR([bsb_io.c])
AM_INIT_AUTOMAKE

AC_CONFIG_MACRO_DIR([m4])

dnl AC_INIT(bsb2tif.c)
dnl AM_INIT_AUTOMAKE(libbsb, 0.0.9)

dnl AC_PROG_CC finds CC and defaults CFLAGS (usually -g -O2)
orig_cflags=$CFLAGS
AC_PROG_CC
AC_PROG_CXX

dnl Check if we can use -flto=thin (i.e. clang).  Otherwise we try -flto.
dnl AX_CHECK_COMPILE_FLAG is in m4/ax_check_compile_flag.m4.  It sholud have been included in aclocal.m4 by running autoreconf or aclocal.
have_flto_thin=no
have_flto=no
have_flto_auto_n=no
have_fuse_linker_plugin=no
AX_CHECK_COMPILE_FLAG([-flto], [have_flto=yes], [have_flto=no])
AX_CHECK_COMPILE_FLAG([-flto=thin], [have_flto_thin=yes], [have_flto_thin=no])
AX_CHECK_COMPILE_FLAG([-flto=auto], [have_flto_auto_n=yes], [have_flto_auto_n=no])
AX_CHECK_LINK_FLAG([-fuse-linker-plugin], [have_fuse_linker_plugin=yes], [have_fuse_linker_plugin=no])

dnl optimize and debug options.  --enable-optimize adds -O3. --enable-debug just adds -g or -Zi. Specifying neither does neither.
dnl TODO use HELP_STRING macro
AC_ARG_ENABLE([optimize], [AS_HELP_STRING([--enable-optimize], [Enable optimization (O3).  If not specified, no optimization flags are added.])])
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [Enable debug information (-g or -Zi).  If not specified, no debug information is included.])])
AC_ARG_ENABLE([lto], [AS_HELP_STRING([--enable-lto], [Enable link-time optimization if --enable-optimization was also given. libbsb itself may not benefit from LTO but when the static library libbsb.a is linked to a program using -flto and -fuse-linker-plugin, then the libbsb objects files may be used by the optimizer.])])

dnl TODO Make sure to use gcc-ranlib or gcc-ar if LTO is enabled and the compiler is GCC?? 

if test x"$enable_optimize" = "xyes"
  then 
    AC_MSG_NOTICE([Optimization enabled])
    if test x"$enable_lto" = "xyes"
    then 
      AC_MSG_NOTICE([Link-time optimization enabled])
    fi
  else 
    AC_MSG_NOTICE([Optimization not enabled])
fi

if test x"$enable_debug" = "xyes"; then AC_MSG_NOTICE([Debug enabled]); else AC_MSG_NOTICE([Debug not enabled]); fi
if test x"$enable_lto" = "xyes"; then AC_MSG_NOTICE([Link-time optimization enabled]); fi

dnl Default CFLAGS for developer's convenience and avoid ranlib with MSVC
use_msvc=no
case "$CC" in
  dnl TODO this will match "clang" (but not /usr/bin/clang or similar), need some other way to detect MSVC cl.exe:
	cl*|CL*)
		# -Od turn off optimisation, -Zi turn on debugging info
		CFLAGS="-nologo -W3"
    if test x"$enable_optimize" = "xyes"; then CFLAGS="$CFLAGS -O3"; else CFLAGS="$CFLAGS -Od"; fi
    if test x"$enable_debug" = "xyes"; then CFLAGS="$CFLAGS -Zi"; fi
    if test x"$enable_optimize" = "xyes" -a x"$enable_lto" = "xyes"; then CFLAGS="$CFLAGS -GL"; LDFLAGS="$LDFLAGS -LTCG"; fi
		# Use a no-op command for ranlib
		RANLIB="true"
		use_msvc=yes
		;;
	*)
		# override AC_PROG_CC default CFLAGS except when user specified CFLAGS
		if test x"$orig_cflags" != x"$CFLAGS" ; then
			CFLAGS="-W -Wall -Werror -fPIC"
		fi
    if test x"$enable_optimize" = "xyes"; then CFLAGS="$CFLAGS -O3"; fi
    if test x"$enable_debug" = "xyes"; then CFLAGS="$CFLAGS -g"; fi
    if test x"$enable_optimize" = "xyes" -a x"$enable_lto" = "xyes"
    then 
      if test x"$have_flto_thin" = "xyes" 
      then 
        CFLAGS="$CFLAGS -flto=thin" 
      else
        if test x"$have_flto" = "xyes"
        then
          if test x"$have_flto_auto_n" = "xyes"
          then
            CFLAGS="$CFLAGS -flto=auto"
          else
            CFLAGS="$CFLAGS -flto"
          fi
          dnl This is needed by the utility programs linking to libbsb.a to use objects from libbsb.a in their own LTO:
          if test x"$have_fuse_linker_plugin" = "xyes"
          then
            LDFLAGS="$LDFLAGS -fuse-linker-plugin"     
          fi
        else
          AC_MSG_ERROR([--enable-optimize and --enable-lto specified, but compiler does not support -flto option.])
        fi
      fi
    fi

    dnl Try use gcc-ranlib and gcc-ar if --enable-lto and have_flto:
		AC_PROG_RANLIB
		;;
esac
AM_CONDITIONAL(USE_MSVC, test x"$use_msvc" = "xyes")


dnl --with options to make it easier to cross compile:
dnl TODO use AS_HELP_STRING macro
AC_ARG_WITH([target], AS_HELP_STRING([--with-target=<target>], [add --target compilation flag with cross-compilation target name. default is to omit --target.]), [CFLAGS="$CFLAGS --target=$withval"], [])
AC_ARG_WITH([toolchain], AS_HELP_STRING([--with-toolchain=<toolchain-file>], [add --gcc-toolchain compilation flag with cross-compilation toolchain file. default is to omit --gcc-toolchain.]), [CFLAGS="$CFLAGS --gcc-toolchain=$withval"], [])
AC_ARG_WITH([sysroot], AS_HELP_STRING([--with-sysroot=<sysroot>], [add --sysroot compilation flag with cross-compilation sysroot path. default is to omit --sysroot.]), [CFLAGS="$CFLAGS --sysroot=$withval"])
AC_ARG_WITH([cc], AS_HELP_STRING([--with-cc=<command>], [Set CC variable with C compiler command.]), [CC=$withval], [])
AC_ARG_WITH([arch], AS_HELP_STRING([--with-arch=<architecture>], [add -march compilation flag with given value. default is to omit -march.]), [CFLAGS="$CFLAGS -march=$withval"], [])


dnl disable building programs, library only
AC_ARG_ENABLE(programs, 
  [AS_HELP_STRING([--disable-programs], [Only build libbsb library. Default (or if --enable-programs given) is to build libbsb library and example/utility conversion programs.])],
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
  AC_ARG_WITH([qt], AS_HELP_STRING([--with-qt=<qtdir>], [use qt4 installed at the given location to build bsbview, if possible. default is to use environment QTDIR variable]), qtdir=$withval, qtdir=${QTDIR})
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
    AC_SUBST([BSBVIEW_INCLUDES], [$QTDIR/include])
    
    dnl now make sure it works
    old_cppflags="$CPPFLAGS"
    export CPPFLAGS="$CPPFLAGS -I$qtdir/include"
    AC_CHECK_HEADER([QtCore/qglobal.h], [have_libqt=yes])
    export CPPFLAGS="$old_cppflags"
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
