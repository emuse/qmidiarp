#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl AC_PREREQ([2.63])
AC_INIT([qmidiarp], [0.5.2], [qmidiarp-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS(src/config.h)
AM_INIT_AUTOMAKE([dist-bzip2])
AC_DEFINE(APP_NAME, "QMidiArp", [application name])

# Initialize doxygen
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN($PACKAGE_NAME, Doxyfile)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# Qt lib check is broken on some distros
# AC_CHECK_LIB([QtCore], qt_addObject, [],
#              [AC_MSG_ERROR(QtCore is required)])
# AC_CHECK_LIB([QtGui], _init, [],
#              [AC_MSG_ERROR(QtGui is required)])

# Checks for libraries
AC_CHECK_LIB([asound], [snd_pcm_open], ,
             [AC_MSG_ERROR(ALSA library (asound) is required)])
AC_CHECK_LIB([jack], [jack_client_open], ,
             [AC_MSG_ERROR(JACK library (jack) is required)])
AC_CHECK_LIB([m], [roundf], , AC_MSG_ERROR([required libm missing]))

PKG_CHECK_MODULES(QT, QtCore QtGui >= 4.2.0, HAVE_QT4="yes", HAVE_QT4="no")
if test "x$HAVE_QT4" = "xyes"; then
  dnl Check needed because in some cases the QtGui includedir
  dnl doesn't contain the subsystem dir.
  QT_INCLUDE_DIR=`$PKG_CONFIG --variable=includedir QtGui`
  QT_CXXFLAGS=`$PKG_CONFIG --cflags QtCore QtGui`
  EXTRA_QT_INCLUDE_DIR="$QT_INCLUDE_DIR/Qt"
  AC_CHECK_FILE([$QT_INCLUDE_DIR/QWidget],
  AC_MSG_NOTICE([No extra QT_INCLUDE_DIR needed]),
  AC_CHECK_FILE([$EXTRA_QT_INCLUDE_DIR/QWidget],
             QT_CPPFLAGS="$QT_CPPFLAGS -I$EXTRA_QT_INCLUDE_DIR",
             AC_MSG_WARN([QWidget not found])))

  AC_MSG_NOTICE([Set QT_CXXFLAGS... $QT_CXXFLAGS])
  AC_SUBST(QT_CXXFLAGS)
else
  AC_MSG_ERROR([cannot find Qt4 library >= 4.2])
fi

dnl check for moc
AC_PATH_PROGS(MOC, moc-qt4 moc, no, [$PATH:$QTDIR/bin])
if test "x$MOC" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'moc', please install the Qt4 development tools package.])
fi

# Enable translations.
AC_ARG_ENABLE(translations,
  AC_HELP_STRING([--enable-translations], [enable translation file generation (default=no)]),
  [ac_translations="$enableval"],
  [ac_translations="no"])

AM_CONDITIONAL([ENABLE_TRANSLATIONS], [test "x$ac_translations" = "xyes"])

AM_COND_IF([ENABLE_TRANSLATIONS],[
dnl check for lupdate
AC_PATH_PROGS(LUPDATE, lupdate-qt4 lupdate, no, [$PATH:$QTDIR/bin])
if test "x$LUPDATE" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'lupdate', please install the Qt4 development tools package.])
fi

dnl check for lrelease
AC_PATH_PROGS(LRELEASE, lrelease-qt4 lrelease, no, [$PATH:$QTDIR/bin])
if test "x$LRELEASE" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'lrelease', please install the Qt4 development tools package.])
fi
], [AC_MSG_NOTICE([translations are disabled])])

# Enable NSM support
AC_ARG_ENABLE(nsm,
  AC_HELP_STRING([--enable-nsm], [enable NSM support generation (default=yes)]),
  [ac_nsm="$enableval"],
  [ac_nsm="yes"])

AM_CONDITIONAL([ENABLE_NSM], [test "x$ac_nsm" = "xyes"])

AM_COND_IF([ENABLE_NSM],[
dnl check for liblo
AC_CHECK_HEADER( lo/lo.h, loinclude_found="yes", loinclude_found="no" )
# OSC support
PKG_CHECK_MODULES(LIBLO, liblo >= 0.26, libloWorks="yes", libloWorks="no")
LIBSsave=$LIBS
CFLAGSsave=$CFLAGS
CFLAGS=$LIBLO_CFLAGS
LIBS=$LIBLO_LIBS
# AC_MSG_CHECKING([for liblo OSC library])
# AC_MSG_RESULT()
# AC_CHECK_LIB(LIBLO,lo_address_new,[],[libloWorks=no])
#echo "liblo OSC library: $LIBLO_LIBS"
AC_SUBST(LIBLO_CFLAGS)
AC_SUBST(LIBLO_LIBS)
LIBS+=$LIBSsave
CFLAGS+=$CFLAGSsave

if test "$loinclude_found" = "no" -o "libloWorks" = "no"; then
  AC_MSG_NOTICE(
    [cannot find a liblo component for building. Will build without nsm support.])
else
    AC_DEFINE(NSM, 1, [Define to enable NSM session manager support])
fi
], [AC_MSG_NOTICE([NSM support is disabled])])

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SSIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([floor ftruncate getcwd memset mkdir pow rint sqrt])


# Checks for header files.

AC_CHECK_HEADER(jack/session.h, jack_session_found="yes", jack_session_found="no")
if test "$jack_session_found" = "yes"; then
    AC_DEFINE(JACK_SESSION, 1, [Define to enable jack session support])
fi


# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile] [examples/Makefile] [src/Makefile])
AC_CONFIG_FILES([src/pixmaps/Makefile] [man/Makefile] [man/fr/Makefile])
AC_CONFIG_FILES([man/de/Makefile])
AC_OUTPUT

echo
if test "x$ac_translations" = "xno" ; then
echo "Translation file generation disabled."
echo "Use ./configure --enable-translations=yes to enable."
else
echo "Translation file generation enabled."
fi
echo
