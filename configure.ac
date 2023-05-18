#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl AC_PREREQ([2.71])
AC_INIT([qmidiarp],[0.7.0],[qmidiarp-devel@lists.sourceforge.net])
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


# Checks for minimal required libraries for all components
AC_CHECK_LIB([m], [roundf], , AC_MSG_ERROR([required libm missing]))


AC_ARG_ENABLE(alsa,
  AS_HELP_STRING([--enable-alsa],[provide ALSA backend option (default=yes)]),
  [ac_alsa="$enableval"],
  [ac_alsa="yes"])
alsalib_found="yes"

# Build LV2 plugins
AC_ARG_ENABLE(lv2plugins,
  AS_HELP_STRING([--enable-lv2plugins],[build LV2 plugins (default=yes)]),
  [ac_lv2plugins="$enableval"],
  [ac_lv2plugins="yes"])

AM_CONDITIONAL([BUILD_LV2], [test "x$ac_lv2plugins" = "xyes"])
ac_lv2willbuild=no

AM_COND_IF([BUILD_LV2],[
dnl check for lv2atom
AC_CHECK_HEADER( lv2/lv2plug.in/ns/ext/atom/atom.h, lv2include_found="yes", lv2include_found="no" )
PKG_CHECK_MODULES(LV2, lv2 >= 1.10, lv2Works="yes", lv2Works="no")
CFLAGSsave=$CFLAGS
CFLAGS=$LV2_CFLAGS
AC_SUBST(LV2_CFLAGS)
CFLAGS="$CFLAGS $CFLAGSsave"


if test $lv2include_found = "no" -o $lv2Works = "no"; then
    AC_MSG_NOTICE(
        [cannot find an lv2 component for building. Will not build LV2 plugins.])
    AM_CONDITIONAL([BUILD_LV2], false)
    AM_CONDITIONAL([BUILD_LV2_UI], false)
else
    ac_lv2willbuild=yes    
fi

], [AC_MSG_NOTICE([LV2 plugins will not be built])])

# Build LV2 plugin Qt UIs ?
AC_ARG_ENABLE(lv2pluginuis,
    AS_HELP_STRING([--enable-lv2pluginuis],[build LV2 plugin UIs (default=yes)]),
  [ac_lv2pluginuis="$enableval"],
  [ac_lv2pluginuis="yes"])

AM_CONDITIONAL([BUILD_LV2_UI], [test "x$ac_lv2pluginuis" = "xyes" -a "x$ac_lv2willbuild" = "xyes"])

# Build LV2 plugin RTK UIs ?
AC_ARG_ENABLE(lv2rtkpluginuis,
    AS_HELP_STRING([--enable-lv2rtkpluginuis],[build LV2 RTK plugin UIs (default=yes)]),
  [ac_lv2rtkpluginuis="$enableval"],
  [ac_lv2rtkpluginuis="yes"])

AM_CONDITIONAL([BUILD_LV2_UI_RTK], [test "x$ac_lv2rtkpluginuis" = "xyes" -a "x$ac_lv2willbuild" = "xyes"])

# Build QMidiArp standalone application ?
AC_ARG_ENABLE(buildapp,
    AS_HELP_STRING([--enable-buildapp],[build standalone application (default=yes)]),
  [ac_buildapp="$enableval"],
  [ac_buildapp="yes"])

AM_CONDITIONAL([BUILD_APP], [test "x$ac_buildapp" = "xyes"])

AC_ARG_ENABLE(qt4, AS_HELP_STRING([--enable-qt4],[force Qt4 instead of Qt5 library (default=no)]),,
                          enable_qt4=no)
                          
if test "x$ac_buildapp" = "xyes" -o ["x$ac_lv2pluginuis" = "xyes"  -a "x$ac_lv2willbuild" = "xyes"]; then
    AC_MSG_NOTICE([QT stuff required for standalone app or LV2 UIs])
    dnl Check for Qt stuff only if we require GUI building
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
          AC_MSG_NOTICE([cannot find Qt4 library >= 4.2])
          AM_CONDITIONAL([BUILD_APP], false)
          AM_CONDITIONAL([BUILD_LV2_UI], false)
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
          AC_MSG_NOTICE([cannot find Qt5 library >= 5.0])
          AM_CONDITIONAL([BUILD_APP], false)
          AM_CONDITIONAL([BUILD_LV2_UI], false)
        fi
        ac_qtversion=qt5
    ])
    dnl endo of qt version test
    
    dnl check for moc
    AC_PATH_PROGS(MOC, moc-$ac_qtversion moc, no, [$PATH:$QTDIR/bin])
    if test "x$MOC" = "xno" ; then
      AC_MSG_ERROR(
        [cannot find 'moc', please install the Qt development tools package.])
    fi
    
    # Enable translations.
    AC_ARG_ENABLE(translations,
      AS_HELP_STRING([--enable-translations],[enable translation file generation (default=no)]),
      [ac_translations="$enableval"],
      [ac_translations="no"])
    
    AM_CONDITIONAL([ENABLE_TRANSLATIONS], [test "x$ac_translations" = "xyes"])
    
    AM_COND_IF([ENABLE_TRANSLATIONS],[
    dnl check for lupdate
    AC_PATH_PROGS(LUPDATE, lupdate-$ac_qtversion lupdate, no, [$PATH:$QTDIR/bin])
    if test "x$LUPDATE" = "xno" ; then
      AC_MSG_ERROR(
        [cannot find 'lupdate', please install the Qt development tools package.])
    fi
    
    dnl check for lrelease
    AC_PATH_PROGS(LRELEASE, lrelease-$ac_qtversion lrelease, no, [$PATH:$QTDIR/bin])
    if test "x$LRELEASE" = "xno" ; then
      AC_MSG_ERROR(
        [cannot find 'lrelease', please install the Qt development tools package.])
    fi
    ], [AC_MSG_NOTICE([translations are disabled])])
    
    # Modify the entries in the LV2 ttl files according to Qt version
    # This has currently no effect since the Qt LV2 UI desriptors were disabled
        
    if test "x$ac_qtversion" = "xqt5"; then
        sed --in-place="" "s/Qt4UI/Qt5UI/g" qmidiarp_arp.lv2/qmidiarp_arp_ui.ttl
        sed --in-place="" "s/Qt4UI/Qt5UI/g" qmidiarp_lfo.lv2/qmidiarp_lfo_ui.ttl
        sed --in-place="" "s/Qt4UI/Qt5UI/g" qmidiarp_seq.lv2/qmidiarp_seq_ui.ttl
    fi
    if test "x$ac_qtversion" = "xqt4"; then
        sed --in-place="" "s/Qt5UI/Qt4UI/g" qmidiarp_arp.lv2/qmidiarp_arp_ui.ttl
        sed --in-place="" "s/Qt5UI/Qt4UI/g" qmidiarp_lfo.lv2/qmidiarp_lfo_ui.ttl
        sed --in-place="" "s/Qt5UI/Qt4UI/g" qmidiarp_seq.lv2/qmidiarp_seq_ui.ttl
    fi
else
    AM_CONDITIONAL([ENABLE_TRANSLATIONS], false)
    AM_CONDITIONAL([NEED_MOC], false)
fi

AM_CONDITIONAL([NEED_MOC], [test "x$ac_buildapp" = "xyes" -o "x$ac_lv2pluginuis" = "xyes"])

# Save minimal required libraries
LIBSmin=$LIBS
LIBS=""

if test "x$ac_buildapp" = "xyes" ; then
    AC_MSG_NOTICE([Application build enabled.])
    
    # Checks done if the standalone app is required
    # Checks for libraries
    if test "x$ac_alsa" = "xyes" ; then
        AC_CHECK_LIB([asound], [snd_pcm_open], , alsalib_found="no")
    fi
    AC_CHECK_LIB([jack], [jack_client_open], ,
                 [AC_MSG_ERROR(JACK library (jack) is required)])
    # Enable NSM support
    AC_ARG_ENABLE(nsm,
      AS_HELP_STRING([--enable-nsm],[enable NSM support generation (default=yes)]),
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
    LIBS="$LIBS $LIBSsave"
    CFLAGS="$CFLAGS $CFLAGSsave"

    if test "$loinclude_found" = "no" -o "libloWorks" = "no"; then
      AC_MSG_NOTICE(
        [cannot find a liblo component for building. Will build without nsm support.])
    else
        AC_DEFINE(NSM, 1, [Define to enable NSM session manager support])
    fi
    ], [AC_MSG_NOTICE([NSM support is disabled])])

else
    AM_CONDITIONAL([ENABLE_NSM], false)
fi

# Use the found libraries only for APPBUILD target
LIBS_APP=$LIBS
AC_SUBST(LIBS_APP)
# And minimal libraries for other targets
LIBS=$LIBSmin

if test "x$ac_lv2rtkpluginuis" = "xyes" -a "x$lv2Works" = "xyes"; then
    PKG_CHECK_MODULES(PANGO, pango >= 0.1, libpangoWorks="yes", libpangoWorks="no")
    AC_SUBST(PANGO_CFLAGS)

    AC_CHECK_HEADER( cairo/cairo.h, cairoinclude_found="yes", cairoinclude_found="no" )
    PKG_CHECK_MODULES(CAIRO, cairo >= 0.1, libcairoWorks="yes", libcairoWorks="no")
    AC_SUBST(CAIRO_CFLAGS)

    PKG_CHECK_MODULES(PANGOCAIRO, pangocairo >= 0.1, libpangocairoWorks="yes", libpangocairoWorks="no")
    AC_SUBST(PANGOCAIRO_CFLAGS)

    PKG_CHECK_MODULES(GL, gl >= 0.1, libglWorks="yes", libglWorks="no")
    AC_SUBST(GL_CFLAGS)

    PKG_CHECK_MODULES(GLU, glu >= 0.1, libgluWorks="yes", libgluWorks="no")

    LIBS=$LIBSmin
    CFLAGS=$CFLAGSsave

    AM_CONDITIONAL([BUILD_LV2_UI_RTK], true)
    if test "x$libpangoWorks" = "xno" -o "x$libcairoWorks" = "xno" ; then
        AM_CONDITIONAL([BUILD_LV2_UI_RTK], false)    
    fi
    if test "x$libpangocairoWorks" = "xno" -a "x$libgluWorks" = "xno" ; then
        AM_CONDITIONAL([BUILD_LV2_UI_RTK], false)    
    fi
    if test "x$libgluWorks" = "xno"; then
        AM_CONDITIONAL([BUILD_LV2_UI_RTK], false)
        echo "Will not build LV2 GL UIs, missing libraries"    
    fi
    AM_COND_IF([BUILD_LV2_UI_RTK],[

        echo "Will build LV2 GL UIs"
        LV2_UI_RTK_LIBS="-lX11 $CAIRO_LIBS $PANGOCAIRO_LIBS $PANGO_LIBS $GLU_LIBS $GL_LIBS"
        LV2_UI_RTK_LDFLAGS="-shared -Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed -pthread"
        
        LV2_UI_RTK_CFLAGS="-g -Wall -Wno-unused-function -DHAVE_LV2_1_8 -I. $GLU_CFLAGS \
        -pthread -DHAVE_IDLE_IFACE -I. -g -Wall -Wno-unused-function -DHAVE_LV2_1_8 $LV2_CFLAGS \
        -msse -msse2 -mfpmath=sse -ffast-math -fomit-frame-pointer -O3 -fno-finite-math-only \
        -DNDEBUG -DVERSION=\\\"0.0.0\\\" -fPIC -fvisibility=hidden $CAIRO_CFLAGS $PANGO_CFLAGS \
        -DUSE_GUI_THREAD -pthread -DUINQHACK=\\\"168162529512469\\\" "
        
        AC_SUBST(LV2_UI_RTK_LIBS)
        AC_SUBST(LV2_UI_RTK_LDFLAGS)
        AC_SUBST(LV2_UI_RTK_CFLAGS)
    ], [AC_MSG_NOTICE([LV2 GL UI build disabled])])
else
    AM_CONDITIONAL([BUILD_LV2_UI_RTK], false)    
    echo "LV2 GL UI build disabled by configuration"    

fi

if test "x$alsalib_found" = "xyes" -a "x$ac_alsa" = "xyes" ; then
    AC_DEFINE(HAVE_ALSA, 1, [Define to enable ALSA backend availabilty])
fi
# Remove the following generated file during configure to avoid need for make clean
rm -f src/mainwindow_moc.cpp

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SSIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_STAT
AC_CHECK_FUNCS([floor ftruncate getcwd memset malloc mkdir pow rint sqrt])


# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile] [examples/Makefile] [src/Makefile])
AC_CONFIG_FILES([src/pixmaps/Makefile] [man/Makefile] [man/fr/Makefile])
AC_CONFIG_FILES([man/de/Makefile])
AC_OUTPUT

if test "x$ac_buildapp" = "xyes" -o "x$ac_lv2pluginuis" = "xyes" ; then
    
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
else
    echo
    echo "Will not use any QT libs"
    
    echo
fi

if test "x$alsalib_found" = "xno" -o "x$ac_alsa" = "xno" ; then
    echo "Will build without ALSA backend."
    echo
fi
echo
