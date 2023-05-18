cmake_minimum_required(VERSION 3.10)
project(qmidiarp VERSION 0.7.0
        DESCRIPTION "MIDI arpeggiator, sequencer and LFO for ALSA and JACK"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set (APP_NAME "QMidiArp")
set (PACKAGE "qmidiarp")
set (PACKAGE_NAME "qmidiarp")
set (PACKAGE_VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "emuse@users.sourceforge.net")
set (PACKAGE_STRING "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set (PACKAGE_TARNAME "qmidiarp")
set (VERSION "${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CONFIG_DEBUG 1)
else ()
    set(CONFIG_DEBUG 0)
endif ()

set (CONFIG_PREFIX "${CMAKE_INSTALL_PREFIX}")
set (CONFIG_LV2DIR "${CMAKE_INSTALL_PREFIX}/lib/lv2")

include (GNUInstallDirs)
set (CONFIG_BINDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_BINDIR}")
set (CONFIG_LIBDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set (CONFIG_DATADIR "${CONFIG_PREFIX}/${CMAKE_INSTALL_DATADIR}")
set (CONFIG_MANDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_MANDIR}")

option (CONFIG_APPBUILD "Build QMidiArp Application (default=yes)" 1)
option (CONFIG_JACK_MIDI "Enable JACK MIDI support (default=yes)" 1)
option (CONFIG_ALSA_MIDI "Enable ALSA MIDI support (default=yes)" 1)

# Enable LV2 plugin build.
option (CONFIG_LV2 "Build LV2 plug-ins (default=yes)" 1)
option (CONFIG_LV2_UI "Build LV2 plug-in UIs with Qt (default=yes)" 1)
option (CONFIG_LV2_UI_RTK  "Build LV2 plug-in UIs with RobTk (default=yes)" 1)

# Enable NSM support.
option (CONFIG_NSM "Enable NSM support (default=yes)" 1)
set (APPBUILD ${CONFIG_APBUILD})

# Enable translations.
option (CONFIG_TRANSLATIONS "Build and install translations (default=no)" 1)
set (APPBUILD ${CONFIG_TRANSLATIONS})

# Fix for new CMAKE_REQUIRED_LIBRARIES policy.
if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW)
endif ()

# Fix for CXX_VISIBILITY_PRESET policy.
if (POLICY CMP0063)
  cmake_policy (SET CMP0063 NEW)
  set (CMAKE_CXX_VISIBILITY_PRESET hidden)
  set (CMAKE_VISIBILITY_INLINES_HIDDEN 1)
endif ()

# Check for Qt
if (CONFIG_APPBUILD OR CONFIG_LV2_UI)
  find_package (QT NAMES Qt5 COMPONENTS Core REQUIRED)
  find_package (Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Xml)
  # Check for lrelease translations
  if (CONFIG_TRANSLATIONS)
    find_program (CONFIG_LRELEASE NAMES "lrelease" DOC "lrelease executable location")
    if (NOT CONFIG_LRELEASE)
        set(CONFIG_TRANSLATIONS  0)
        message("Translation generation disabled, lrelease not found")
    else()
        find_program (CONFIG_LUPDATE NAMES "lupdate" DOC "lupdate executable location")
        message("lrelease found: ${CONFIG_LRELEASE}")
        message("Will build translations")
    endif()
  endif()
endif ()

#find_package (Qt${QT_VERSION_MAJOR}LinguistTools)

include (CheckIncludeFileCXX)
include (CheckIncludeFiles)
include (CheckIncludeFile)
include (CheckFunctionExists)
include (CheckLibraryExists)

# Checks for header files.
if (UNIX AND NOT APPLE)
  check_include_files ("fcntl.h;unistd.h;signal.h" HAVE_SIGNAL_H)
endif ()

# Find package modules
find_package (PkgConfig REQUIRED)

if (NOT CONFIG_APPBUILD)
  set (CONFIG_JACK_MIDI 0)
  set (CONFIG_ALSA_MIDI 0)
  set (CONFIG_NSM 0)
endif ()

    
# Check for liblo
if (CONFIG_NSM)
  pkg_check_modules (LIBLO IMPORTED_TARGET liblo)
  if (LIBLO_FOUND)
    include_directories (${LIBLO_INCLUDE_DIRS})
    link_directories (${LIBLO_LIBRARY_DIRS})
    link_libraries (${LIBLO_LIBRARIES})
    set (NSM 1)
  else ()
    message (WARNING "*** LIBLO library not found. Deactivating NSM support")
    set (CONFIG_LIBLO 0)
  endif ()
endif ()

# Check for JACK libraries.
if (CONFIG_APPBUILD)
  pkg_check_modules (JACK IMPORTED_TARGET jack>=0.100.0)
  if (JACK_FOUND)
    include_directories (${JACK_INCLUDE_DIRS})
    link_directories (${JACK_LIBRARY_DIRS})
  # link_libraries (${JACK_LIBRARIES})
    set (CMAKE_REQUIRED_LIBRARIES "${JACK_LIBRARIES};${CMAKE_REQUIRED_LIBRARIES}")
    # Check for JACK MIDI headers availability.
    if (CONFIG_JACK_MIDI)
      check_include_files (jack/midiport.h HAVE_JACK_MIDIPORT_H)
      if (NOT HAVE_JACK_MIDIPORT_H)
        set (HAVE_JACK 0)
      else ()
        set (HAVE_JACK 1)
        set (APPBUILD 1)
      endif ()
    endif ()
    # Check for ALSA libraries.

    if (CONFIG_ALSA_MIDI)
      pkg_check_modules (ALSA IMPORTED_TARGET alsa)
      set (CONFIG_ALSA_MIDI ${ALSA_FOUND})
      if (CONFIG_ALSA_MIDI)
        include_directories (${ALSA_INCLUDE_DIRS})
        link_directories (${ALSA_LIBRARY_DIRS})
        link_libraries (${ALSA_LIBRARIES})
        set (HAVE_ALSA 1)
        set (APPBUILD 1)
        message ("Will build QMidiArp application")
      else ()
        set (HAVE_ALSA 0)
        if (NOT HAVE_JACK)
          set (APPBUILD 0)
            message (WARNING "*** Cannot build application without backend driver.")
        endif ()
        message (WARNING "*** ALSA library not found.")
      endif ()
    endif ()
  else ()
    message (WARNING "*** JACK library not found.")
    set (CONFIG_JACK 0)
  endif ()
endif ()

# Check for LV2 support.
if (CONFIG_LV2)
  pkg_check_modules (LV2 lv2)
  if (LV2_FOUND)
    include_directories (${LV2_INCLUDE_DIRS})
    link_directories (${LV2_LIBRARY_DIRS})
    link_libraries (${LV2_LIBRARIES})
    # Check for LV2 Atom support.
    check_include_file (lv2/lv2plug.in/ns/ext/atom/atom.h HAVE_LV2_ATOM_H)
    if (NOT HAVE_LV2_ATOM_H)
      set (CONFIG_LV2_ATOM 0)
    else ()
      set (CONFIG_LV2_ATOM 1)
      set (BUILD_LV2 1)
      message ("Will build LV2 plugins")
    endif ()
    set (CONFIG_LV2_ATOM_FORGE_OBJECT ${CONFIG_LV2_ATOM})
    set (CONFIG_LV2_ATOM_FORGE_KEY ${CONFIG_LV2_ATOM})
    # Check for LV2 UI support.
    check_include_file (lv2/lv2plug.in/ns/extensions/ui/ui.h HAVE_LV2_UI_H)
    if (NOT HAVE_LV2_UI_H)
      set (CONFIG_LV2_UI 0)
    else ()
      set (CONFIG_LV2_UI 1)
    endif ()
  else ()
    message (WARNING "*** LV2 headers not found.")
    set (CONFIG_LV2 0)
  endif ()
endif ()

if (CONFIG_LV2_UI_RTK)
  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/robtk/ui_gl.c")
    set(CONFIG_LV2_UI_RTK, false)
    message(WARNING "*** robtk not found in source tree. Did you forget git submodule update --init ?")
    message("Will not build RTK LV2 UIs.")
  endif()
  pkg_check_modules (LV2 lv2)
  if (LV2_FOUND)
    include_directories (${LV2_INCLUDE_DIRS})
    link_directories (${LV2_LIBRARY_DIRS})
    link_libraries (${LV2_LIBRARIES})
    # Check for LV2 Atom support.
    check_include_file (lv2/lv2plug.in/ns/ext/atom/atom.h HAVE_LV2_ATOM_H)
    if (NOT HAVE_LV2_ATOM_H)
      set (CONFIG_LV2_ATOM 0)
    else ()
      set (CONFIG_LV2_ATOM 1)
      set (BUILD_LV2_UI_RTK 1)
    endif ()
    set (CONFIG_LV2_ATOM_FORGE_OBJECT ${CONFIG_LV2_ATOM})
    set (CONFIG_LV2_ATOM_FORGE_KEY ${CONFIG_LV2_ATOM})
    # Check for LV2 UI support.
    check_include_file (lv2/lv2plug.in/ns/extensions/ui/ui.h HAVE_LV2_UI_H)
    if (NOT HAVE_LV2_UI_H)
      set (CONFIG_LV2_UI_RTK 0)
    else ()
      set (CONFIG_LV2_UI_RTK 1)
    endif ()
  else ()
    message (WARNING "*** LV2 headers not found.")
    set (CONFIG_LV2_UI_RTK 0)
  endif ()

  pkg_check_modules (PANGO pango)
  if (PANGO_FOUND)
    include_directories (${PANGO_INCLUDE_DIRS})
    link_directories (${PANGO_LIBRARY_DIRS})
    link_libraries (${PANGO_LIBRARIES})
  else ()
    message (WARNING "*** PANGO headers not found.")
    set (CONFIG_LV2_UI_RTK 0)
  endif ()
  
  pkg_check_modules (CAIRO cairo)
  if (PANGO_FOUND)
    include_directories (${CAIRO_INCLUDE_DIRS})
    link_directories (${CAIRO_LIBRARY_DIRS})
    link_libraries (${CAIRO_LIBRARIES})
  else ()
    message (WARNING "*** CAIRO headers not found.")
    set (CONFIG_LV2_UI_RTK 0)
  endif ()
   
  pkg_check_modules (PANGOCAIRO pangocairo)
  if (PANGOCAIRO_FOUND)
    include_directories (${PANGOCAIRO_INCLUDE_DIRS})
    link_directories (${PANGOCAIRO_LIBRARY_DIRS})
    link_libraries (${PANGOCAIRO_LIBRARIES})
  else ()
    message (WARNING "*** PANGOCAIRO headers not found.")
    set (CONFIG_LV2_UI_RTK 0)
  endif ()
  
  pkg_check_modules (GLU glu)
  if (GLU_FOUND)
    include_directories (${GLU_INCLUDE_DIRS})
    link_directories (${GLU_LIBRARY_DIRS})
    link_libraries (${GLU_LIBRARIES})
  else ()
    message (WARNING "*** GLU headers not found.")
    set (CONFIG_LV2_UI_RTK 0)
  endif ()
  
  pkg_check_modules (GL gl)
  if (GL_FOUND)
    include_directories (${GL_INCLUDE_DIRS})
    link_directories (${GL_LIBRARY_DIRS})
    link_libraries (${GL_LIBRARIES})
  else ()
    message (WARNING "*** GL headers not found.")
    set (CONFIG_LV2_UI_RTK 0)
  endif ()
endif ()

if (BUILD_LV2 AND CONFIG_LV2_UI AND QT_FOUND)
    message ("Will build LV2 UIs\n")
    set(BUILD_LV2_UI 1)
endif ()

if (BUILD_LV2 AND CONFIG_LV2_UI_RTK)
    message ("Will build RobTk LV2 UIs\n")
    set(BUILD_LV2_UI_RTK 1)
endif ()

add_subdirectory(src)
add_subdirectory(qmidiarp_arp.lv2)
add_subdirectory(qmidiarp_lfo.lv2)
add_subdirectory(qmidiarp_seq.lv2)

if (EXISTS ${CMAKE_SOURCE_DIR}/configure)
    set(ADD_AUTOCONF_FILES  --add-file=${CMAKE_SOURCE_DIR}/configure
                            --add-file=${CMAKE_SOURCE_DIR}/aclocal.m4
                            --add-file=${CMAKE_SOURCE_DIR}/compile
                            --add-file=${CMAKE_SOURCE_DIR}/config.guess
                            --add-file=${CMAKE_SOURCE_DIR}/config.sub
                            --add-file=${CMAKE_SOURCE_DIR}/depcomp
                            --add-file=${CMAKE_SOURCE_DIR}/install-sh
                            --add-file=${CMAKE_SOURCE_DIR}/ltmain.sh
                            --add-file=${CMAKE_SOURCE_DIR}/missing
                            --add-file=${CMAKE_SOURCE_DIR}/Makefile.in
                            --prefix=${PACKAGE_STRING}/m4/
                            --add-file=${CMAKE_SOURCE_DIR}/m4/libtool.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/ltoptions.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/ltsugar.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/ltversion.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/lt~obsolete.m4
                            --prefix=${PACKAGE_STRING}/man/
                            --add-file=${CMAKE_SOURCE_DIR}/man/Makefile.in
                            --prefix=${PACKAGE_STRING}/man/de/
                            --add-file=${CMAKE_SOURCE_DIR}/man//de/Makefile.in
                            --prefix=${PACKAGE_STRING}/man/fr/
                            --add-file=${CMAKE_SOURCE_DIR}/man//fr/Makefile.in
                            --prefix=${PACKAGE_STRING}/examples/
                            --add-file=${CMAKE_SOURCE_DIR}/examples/Makefile.in
                            --prefix=${PACKAGE_STRING}/src/
                            --add-file=${CMAKE_SOURCE_DIR}/src/config.h.in
                            --add-file=${CMAKE_SOURCE_DIR}/src/Makefile.in
                            --prefix=${PACKAGE_STRING}/src/pixmaps/
                            --add-file=${CMAKE_SOURCE_DIR}/src/pixmaps/Makefile.in
                            --prefix=${PACKAGE_STRING}/)

else ()
    set(ADD_AUTOCONF_FILES )
    message("Warning: No configure file found, will be missing when creating a dist archive")
endif ()

set(DIST_TARCOMMAND git --git-dir=${CMAKE_SOURCE_DIR}/.git archive 
                        --format=tar 
                        --prefix=${PACKAGE_STRING}/ ${ADD_AUTOCONF_FILES}
                        ${PACKAGE_STRING}^{tree} -o ${PACKAGE_STRING}.tar &&
                        git --git-dir=${CMAKE_SOURCE_DIR}/robtk/.git archive 
                        --format=tar 
                        --prefix=${PACKAGE_STRING}/robtk/ HEAD -o robtk.tar &&
                        tar --concatenate --file=${PACKAGE_STRING}.tar robtk.tar && 
                        rm -f robtk.tar)

add_custom_target(dist-bz2 COMMAND ${DIST_TARCOMMAND} && bzip2 -f ${PACKAGE_STRING}.tar )
add_custom_target(dist COMMAND ${DIST_TARCOMMAND} && gzip -f ${PACKAGE_STRING}.tar )


install (FILES man/qmidiarp.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install (FILES man/fr/qmidiarp.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/fr/man1)
install (FILES man/de/qmidiarp.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/de/man1)
install (FILES ${PROJECT_NAME}.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install (FILES ${PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install (FILES ${PROJECT_NAME}.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
