#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl AC_PREREQ([2.63])
AC_INIT([qmidiarp], [0.6.2], [qmidiarp-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_MACRO_DIR([m4])
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

AC_ENABLE_SHARED
AC_DISABLE_STATIC
LT_INIT([dlopen])


# Checks for libraries
AC_CHECK_LIB([asound], [snd_pcm_open], ,
             [AC_MSG_ERROR(ALSA library (asound) is required)])
AC_CHECK_LIB([jack], [jack_client_open], ,
             [AC_MSG_ERROR(JACK library (jack) is required)])
AC_CHECK_LIB([m], [roundf], , AC_MSG_ERROR([required libm missing]))

AC_ARG_ENABLE(qt5, AC_HELP_STRING([--enable-qt5],
                          [enable Qt5 instead of Qt4 library]),,
                          enable_qt4=yes)

AS_IF([test x$enable_qt4 = xyes],
dnl Check for Qt4 library files
[
PKG_CHECK_MODULES(Qt4, QtCore QtGui >= 4.2.0, HAVE_QT4="yes", HAVE_QT4="no")
if test "x$HAVE_QT4" = "xyes"; then
  dnl Check needed because in some cases the QtGui includedir
  dnl does not contain the subsystem dir.
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
  AC_SUBST(Qt4_LIBS)
else
  AC_MSG_ERROR([cannot find Qt4 library >= 4.2])
fi
ac_qtversion=qt4],
dnl ... or use Qt5
[
PKG_CHECK_MODULES(Qt5, Qt5Core Qt5Gui Qt5Widgets >= 5.0,
                  HAVE_QT5="yes", HAVE_QT5="no")
if test "x$HAVE_QT5" = "xyes"; then
  dnl Check needed because in some cases the QtGui includedir
  dnl doesnt contain the subsystem dir.
  QT_INCLUDE_DIR=`$PKG_CONFIG --variable=includedir Qt5Gui`
  QT_CXXFLAGS=`$PKG_CONFIG --cflags Qt5Core Qt5Gui Qt5Widgets`
  QT_CXXFLAGS="$QT_CXXFLAGS -fPIC"
  EXTRA_QT_INCLUDE_DIR="$QT_INCLUDE_DIR/Qt"
  AC_CHECK_FILE([$QT_INCLUDE_DIR/QtWidgets/QWidget],
    AC_MSG_NOTICE([No extra QT_INCLUDE_DIR needed]),
      AC_CHECK_FILE([$EXTRA_QT_INCLUDE_DIR/QtWidgets/QWidget],
        QT_CPPFLAGS="$QT_CPPFLAGS -I$EXTRA_QT_INCLUDE_DIR",
          AC_MSG_WARN([QWidget not found])))

  AC_SUBST(QT_CXXFLAGS)
  AC_SUBST(Qt5_LIBS)
  AC_DEFINE([HAVE_QT5], [1],[Define to 1 if you have Qt5.])

else
  AC_MSG_ERROR([cannot find Qt5 library >= 5.0])
fi
ac_qtversion=qt5
])
dnl endo of qt version test

dnl check for moc
AC_PATH_PROGS(MOC, moc-$ac_qtversion moc, no, [$PATH:$QTDIR/bin])
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
AC_PATH_PROGS(LUPDATE, lupdate-$ac_qtversion lupdate, no, [$PATH:$QTDIR/bin])
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

# Build LV2 plugins
AC_ARG_ENABLE(lv2plugins,
  AC_HELP_STRING([--enable-lv2plugins], [build LV2 plugins (default=yes)]),
  [ac_lv2plugins="$enableval"],
  [ac_lv2plugins="yes"])

AM_CONDITIONAL([BUILD_LV2], [test "x$ac_lv2plugins" = "xyes"])

AM_COND_IF([BUILD_LV2],[
dnl check for lv2atom
AC_CHECK_HEADER( lv2/lv2plug.in/ns/ext/atom/atom.h, lv2include_found="yes", lv2include_found="no" )
PKG_CHECK_MODULES(LV2, lv2 >= 1.10, lv2Works="yes", lv2Works="no")
CFLAGSsave=$CFLAGS
CFLAGS=$LV2_CFLAGS
AC_SUBST(LV2_CFLAGS)
CFLAGS+=$CFLAGSsave

if test $lv2include_found = "no" -o $lv2Works = "no"; then
    AC_MSG_NOTICE(
        [cannot find an lv2 component for building. Will not build LV2 plugins.])
    AM_CONDITIONAL([BUILD_LV2], 0)
fi

], [AC_MSG_NOTICE([LV2 plugins will not be built])])

# Remove the following generated file during configure to avoid need for make clean
rm -f src/mainwindow_moc.cpp

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
echo "Will use the following QT libs:" $Qt4_LIBS $Qt5_LIBS

echo
if test "x$ac_translations" = "xno" ; then
    echo "Translation file generation disabled."
    echo "Use ./configure --enable-translations=yes to enable."
else
    echo "Translation file generation enabled."
fi
echo
