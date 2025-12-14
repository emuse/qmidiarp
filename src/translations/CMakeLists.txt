set (TS_SOURCES 
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_cs.ts
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_de.ts
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_es.ts
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_fr.ts
    )

set (QM_FILES 
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_cs.qm
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_de.qm
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_es.qm
    ${CMAKE_SOURCE_DIR}/src/translations/${PACKAGE}_fr.qm
    )

# add_custom_target(lupdate ALL COMMAND ${CONFIG_LUPDATE} ${CMAKE_SOURCE_DIR}/src -ts ${TS_SOURCES})

add_custom_target(lrelease ALL COMMAND Qt${QT_VERSION_MAJOR}::lrelease ${TS_SOURCES})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PACKAGE}/translations/)
