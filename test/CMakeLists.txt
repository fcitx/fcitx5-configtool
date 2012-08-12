include_directories(${PROJECT_SOURCE_DIR}/src)

kde4_add_executable(testconfigpage
    testconfigpage.cpp
    ../src/configdescmanager.cpp
    ../src/configwidget.cpp
    ../src/subconfig.cpp
    ../src/subconfigparser.cpp
    ../src/subconfigpattern.cpp
    ../src/subconfigwidget.cpp
    ../src/qtkeytrans.cpp
    ../src/dummyconfig.cpp
    ../src/verticalscrollarea.cpp
)

target_link_libraries(testconfigpage
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${FCITX4_FCITX_UTILS_LIBRARIES}
    ${FCITX4_FCITX_CONFIG_LIBRARIES}
)

kde4_add_executable(testdummyconfig
    testdummyconfig.cpp
    ../src/configdescmanager.cpp
    ../src/dummyconfig.cpp
    ../src/verticalscrollarea.cpp
)

target_link_libraries(testdummyconfig
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${FCITX4_FCITX_UTILS_LIBRARIES}
    ${FCITX4_FCITX_CONFIG_LIBRARIES}
)