find_package(Qt4 4.7.0 REQUIRED)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})

set(kcm_SRCS
    main.cpp
    Module.cpp
    FcitxAddonSelector.cpp
    FcitxConfigPage.cpp
    ConfigDescManager.cpp
)

kde4_add_ui_files(kcm_SRCS Module.ui FcitxConfigPage.ui)

kde4_add_plugin(kcm_fcitx ${kcm_SRCS})

target_link_libraries(kcm_fcitx
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${FCITX_CONFIG_LIBRARIES}
    ${FCITX_LIBRARIES}
)

install(TARGETS kcm_fcitx DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_fcitx.desktop DESTINATION ${SERVICES_INSTALL_DIR})
