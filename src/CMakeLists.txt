set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
endif()

configure_file (cmake_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

if (APPBUILD)
    set (HEADERS
        cursor.h
        engine.h
        arpscreen.h
        lfoscreen.h
        seqscreen.h
        arpwidget.h
        lfowidget.h 
        seqwidget.h 
        groovewidget.h 
        mainwindow.h 
        globstore.h 
        indicator.h 
        modulewidget.h 
        logwidget.h 
        main.h 
        midiworker.h 
        midiarp.h 
        midilfo.h 
        midiseq.h 
        midicctable.h 
        midicontrol.h 
        midievent.h 
        nsm.h 
        driverbase.h 
        parstore.h 
        prefswidget.h 
        prefs.h 
        jackdriver.h 
        screen.h 
        seqdriver.h 
        slider.h 
        storagebutton.h
        )
    
    set( SOURCES
        cursor.cpp
        engine.cpp
        arpscreen.cpp
        lfoscreen.cpp
        seqscreen.cpp
        arpwidget.cpp
        lfowidget.cpp
        seqwidget.cpp
        groovewidget.cpp
        mainwindow.cpp
        globstore.cpp
        indicator.cpp
        modulewidget.cpp
        logwidget.cpp
        main.cpp
        midiworker.cpp
        midiarp.cpp
        midilfo.cpp
        midiseq.cpp
        midicctable.cpp
        midicontrol.cpp
        parstore.cpp
        prefswidget.cpp
        prefs.cpp
        jackdriver.cpp
        screen.cpp
        seqdriver.cpp
        slider.cpp
        storagebutton.cpp
    )
    set( DTRANSLATIONSDIR "")
    
    if (CONFIG_TRANSLATIONS)
        add_subdirectory(translations)
        set( DTRANSLATIONSDIR -DTRANSLATIONSDIR="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PACKAGE}/translations/" )
    endif()
    
    add_executable (${PROJECT_NAME}
      ${HEADERS}
      ${SOURCES}
    )

    set_target_properties (${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)
    target_compile_options(${PROJECT_NAME} PRIVATE -DAPPBUILD ${DTRANSLATIONSDIR})

    if (CONFIG_JACK_MIDI)
      target_link_libraries (${PROJECT_NAME} PRIVATE PkgConfig::JACK)
    endif ()
    
    if (CONFIG_ALSA_MIDI)
      target_link_libraries (${PROJECT_NAME} PRIVATE PkgConfig::ALSA)
    endif ()
    
    target_link_libraries (${PROJECT_NAME} PUBLIC   Qt${QT_VERSION_MAJOR}::Core 
                                                    Qt${QT_VERSION_MAJOR}::Gui
                                                    Qt${QT_VERSION_MAJOR}::Widgets
                                                    )
    
    if (UNIX AND NOT APPLE AND NOT CONFIG_DEBUG)
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND strip ${PROJECT_NAME})
    endif ()

    install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

endif ()
#LV2 DSPs

if (BUILD_LV2)
    set(HEADERS_arp
        lv2_common.h
        main.h
        midiworker.h
        midiarp.h
        midiarp_lv2.h
    )
    
    set(SOURCES_arp
        midiworker.cpp
        midiarp.cpp
        midiarp_lv2.cpp
    )
    
    set(LV2_LFO_HEADERS
        lv2_common.h
        main.h
        midiworker.h
        midilfo.h
        midilfo_lv2.h
    )
    
    set(LV2_LFO_SOURCES
        midiworker.cpp
        midilfo.cpp
        midilfo_lv2.cpp
    )
    
    set(LV2_SEQ_HEADERS
        lv2_common.h
        main.h
        midiworker.h
        midiseq.h
        midiseq_lv2.h
    )
    
    set(LV2_SEQ_SOURCES
        midiworker.cpp
        midiseq.cpp
        midiseq_lv2.cpp
    )
    
    add_library (${PROJECT_NAME}_arp MODULE
      ${HEADERS_arp}
      ${SOURCES_arp}
    )
    
    add_library (${PROJECT_NAME}_lfo MODULE
      ${LV2_LFO_HEADERS}
      ${LV2_LFO_SOURCES}
    )
    
    add_library (${PROJECT_NAME}_seq MODULE
      ${LV2_SEQ_HEADERS}
      ${LV2_SEQ_SOURCES}
    )

    set_target_properties (${PROJECT_NAME}_arp PROPERTIES CXX_STANDARD 11 PREFIX "")
    set_target_properties (${PROJECT_NAME}_lfo PROPERTIES CXX_STANDARD 11 PREFIX "")
    set_target_properties (${PROJECT_NAME}_seq PROPERTIES CXX_STANDARD 11 PREFIX "")

    if (UNIX AND NOT APPLE AND NOT CONFIG_DEBUG)
      add_custom_command(TARGET ${PROJECT_NAME}_arp  POST_BUILD
        COMMAND strip ${PROJECT_NAME}_arp.so)
      add_custom_command(TARGET ${PROJECT_NAME}_lfo  POST_BUILD
        COMMAND strip ${PROJECT_NAME}_lfo.so)
      add_custom_command(TARGET ${PROJECT_NAME}_seq  POST_BUILD
        COMMAND strip ${PROJECT_NAME}_seq.so)
    endif ()

    if (UNIX AND NOT APPLE)
      if (CONFIG_PREFIX MATCHES $ENV{HOME})
        set (CONFIG_LV2DIR ${CONFIG_PREFIX}/.lv2)
      endif ()
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_arp.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_arp.lv2)
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_lfo.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_lfo.lv2)
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_seq.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_seq.lv2)
    endif ()
endif ()

# LV2 UIs
if (BUILD_LV2_UI)
    
    set(LV2UI_ARP_HEADERS
        cursor.h
        lv2_common.h
        main.h
        midiworker.h
        midiarp.h
        modulewidget.h
        arpwidget.h
        arpscreen.h
        screen.h
        slider.h
        arpwidget_lv2.h
    )
    
    set(LV2UI_ARP_SOURCES
        cursor.cpp
        midiworker.cpp
        midiarp.cpp
        modulewidget.cpp
        arpwidget.cpp
        arpscreen.cpp
        screen.cpp
        slider.cpp
        arpwidget_lv2.cpp
    )
    
    set(LV2UI_LFO_HEADERS
        cursor.h
        lv2_common.h
        main.h
        midiworker.h
        midilfo.h
        modulewidget.h
        lfowidget.h
        lfoscreen.h
        screen.h
        slider.h
        lfowidget_lv2.h
    )
    
    set(LV2UI_LFO_SOURCES
        cursor.cpp
        midiworker.cpp
        midilfo.cpp
        modulewidget.cpp
        lfowidget.cpp
        lfoscreen.cpp
        screen.cpp
        slider.cpp
        lfowidget_lv2.cpp
    )
    
    set(LV2UI_SEQ_HEADERS
        cursor.h
        lv2_common.h
        main.h
        midiworker.h
        midiseq.h
        modulewidget.h
        seqwidget.h
        seqscreen.h
        screen.h
        slider.h
        seqwidget_lv2.h
    )
    
    set(LV2UI_SEQ_SOURCES
        cursor.cpp
        midiworker.cpp
        midiseq.cpp
        modulewidget.cpp
        seqwidget.cpp
        seqscreen.cpp
        screen.cpp
        slider.cpp
        seqwidget_lv2.cpp
    )
    
    add_library (${PROJECT_NAME}_arp_ui MODULE
      ${LV2UI_ARP_HEADERS}
      ${LV2UI_ARP_SOURCES}
    )
    
    add_library (${PROJECT_NAME}_lfo_ui MODULE
      ${LV2UI_LFO_HEADERS}
      ${LV2UI_LFO_SOURCES}
    )
    
    add_library (${PROJECT_NAME}_seq_ui MODULE
      ${LV2UI_SEQ_HEADERS}
      ${LV2UI_SEQ_SOURCES}
    )

    set_target_properties (${PROJECT_NAME}_arp_ui PROPERTIES CXX_STANDARD 11 PREFIX "")
    set_target_properties (${PROJECT_NAME}_lfo_ui PROPERTIES CXX_STANDARD 11 PREFIX "")
    set_target_properties (${PROJECT_NAME}_seq_ui PROPERTIES CXX_STANDARD 11 PREFIX "")
    
    target_link_libraries (${PROJECT_NAME}_arp_ui PUBLIC   Qt${QT_VERSION_MAJOR}::Core 
                                                    Qt${QT_VERSION_MAJOR}::Gui
                                                    Qt${QT_VERSION_MAJOR}::Widgets
                                                    )
    
    target_link_libraries (${PROJECT_NAME}_lfo_ui PUBLIC   Qt${QT_VERSION_MAJOR}::Core 
                                                    Qt${QT_VERSION_MAJOR}::Gui
                                                    Qt${QT_VERSION_MAJOR}::Widgets
                                                    )
    
    target_link_libraries (${PROJECT_NAME}_seq_ui PUBLIC   Qt${QT_VERSION_MAJOR}::Core 
                                                    Qt${QT_VERSION_MAJOR}::Gui
                                                    Qt${QT_VERSION_MAJOR}::Widgets
                                                    )
    if (UNIX AND NOT APPLE AND NOT CONFIG_DEBUG)
      add_custom_command(TARGET ${PROJECT_NAME}_arp_ui  POST_BUILD
        COMMAND strip ${PROJECT_NAME}_arp_ui.so)
      add_custom_command(TARGET ${PROJECT_NAME}_lfo_ui  POST_BUILD
        COMMAND strip ${PROJECT_NAME}_lfo_ui.so)
      add_custom_command(TARGET ${PROJECT_NAME}_seq_ui  POST_BUILD
        COMMAND strip ${PROJECT_NAME}_seq_ui.so)
    endif ()

    if (UNIX AND NOT APPLE)
      if (CONFIG_PREFIX MATCHES $ENV{HOME})
        set (CONFIG_LV2DIR ${CONFIG_PREFIX}/.lv2)
      endif ()
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_arp_ui.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_arp.lv2)
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_lfo_ui.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_lfo.lv2)
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_seq_ui.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_seq.lv2)
    endif ()
    
endif()

# LV2 RobTk UIs
if (BUILD_LV2_UI_RTK)
    
    set(LV2UI_RTK_ARP_SOURCES
        ../robtk/pugl/pugl_x11.c
        ../robtk/ui_gl.c 
    )
  
    add_library (${PROJECT_NAME}_arp_ui_gl MODULE
      ${LV2UI_RTK_ARP_HEADERS}
      ${LV2UI_RTK_ARP_SOURCES}
    )

    set_target_properties (${PROJECT_NAME}_arp_ui_gl PROPERTIES C_STANDARD 11 PREFIX "")
    target_compile_options(${PROJECT_NAME}_arp_ui_gl PRIVATE   -g -Wall 
                                    -Wno-unused-function -fvisibility=hidden -DHAVE_LV2_1_8
                                    -DUSE_GUI_THREAD -pthread -DUINQHACK="168162529512469" 
                                    -DPLUGIN_SOURCE="arpwidget_lv2_rtk.c"
                                    -g -Wall -Wno-unused-function 
                                    -DHAVE_LV2_1_8 -I.. -I. 
                                    -pthread -DHAVE_IDLE_IFACE -I.
                                    )
    
    target_link_libraries (${PROJECT_NAME}_arp_ui_gl PUBLIC -lX11)

    set(LV2UI_RTK_LFO_SOURCES
        ../robtk/pugl/pugl_x11.c
        ../robtk/ui_gl.c 
    )
  
    add_library (${PROJECT_NAME}_lfo_ui_gl MODULE
      ${LV2UI_RTK_LFO_HEADERS}
      ${LV2UI_RTK_LFO_SOURCES}
    )

    set_target_properties (${PROJECT_NAME}_lfo_ui_gl PROPERTIES C_STANDARD 11 PREFIX "")
    target_compile_options(${PROJECT_NAME}_lfo_ui_gl PRIVATE   -g -Wall 
                                    -Wno-unused-function -fvisibility=hidden -DHAVE_LV2_1_8
                                    -DUSE_GUI_THREAD -pthread -DUINQHACK="168162529512469" 
                                    -DPLUGIN_SOURCE="lfowidget_lv2_rtk.c"
                                    -g -Wall -Wno-unused-function 
                                    -DHAVE_LV2_1_8 -I.. -I. 
                                    -pthread -DHAVE_IDLE_IFACE -I.
                                    )
    
    target_link_libraries (${PROJECT_NAME}_lfo_ui_gl PUBLIC -lX11)

    set(LV2UI_RTK_SEQ_SOURCES
        ../robtk/pugl/pugl_x11.c
        ../robtk/ui_gl.c 
    )
  
    add_library (${PROJECT_NAME}_seq_ui_gl MODULE
      ${LV2UI_RTK_SEQ_HEADERS}
      ${LV2UI_RTK_SEQ_SOURCES}
    )

    set_target_properties (${PROJECT_NAME}_seq_ui_gl PROPERTIES C_STANDARD 11 PREFIX "")
    target_compile_options(${PROJECT_NAME}_seq_ui_gl PRIVATE   -g -Wall 
                                    -Wno-unused-function -fvisibility=hidden -DHAVE_LV2_1_8
                                    -DUSE_GUI_THREAD -pthread -DUINQHACK="168162529512469" 
                                    -DPLUGIN_SOURCE="seqwidget_lv2_rtk.c"
                                    -g -Wall -Wno-unused-function 
                                    -DHAVE_LV2_1_8 -I.. -I. 
                                    -pthread -DHAVE_IDLE_IFACE -I.
                                    )
    
    target_link_libraries (${PROJECT_NAME}_seq_ui_gl PUBLIC -lX11)

    if (UNIX AND NOT APPLE)
      if (CONFIG_PREFIX MATCHES $ENV{HOME})
        set (CONFIG_LV2DIR ${CONFIG_PREFIX}/.lv2)
      endif ()
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_arp_ui_gl.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_arp.lv2)
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_lfo_ui_gl.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_lfo.lv2)
      install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_seq_ui_gl.so
         DESTINATION ${CONFIG_LV2DIR}/${PROJECT_NAME}_seq.lv2)
    endif ()

endif()

