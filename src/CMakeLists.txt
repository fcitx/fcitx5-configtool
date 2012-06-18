include_directories(
    ${FCITX4_FCITX_INCLUDE_DIRS}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
    ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
    )

link_directories(
    ${FCITX4_FCITX_LIBRARY_DIRS}
    ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
    ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
    )

set(kcm_SRCS
    main.cpp
    Module.cpp
    FcitxAddonSelector.cpp
    FcitxConfigPage.cpp
    ConfigDescManager.cpp
    FcitxSubConfigParser.cpp
    FcitxSubConfigPattern.cpp
    FcitxSubConfig.cpp
    FcitxSubConfigWidget.cpp
    FcitxSkinPage.cpp
    FcitxIMPage.cpp
    FcitxIM.cpp
    FcitxLayout.cpp
    FcitxIMConfigDialog.cpp
)

set_property(
    SOURCE org.fcitx.Fcitx.InputMethod.xml
    PROPERTY INCLUDE FcitxIM.h
)

set_property(
    SOURCE org.fcitx.Fcitx.Keyboard.xml
    PROPERTY INCLUDE FcitxLayout.h
)

QT4_ADD_DBUS_INTERFACE(kcm_SRCS
    org.fcitx.Fcitx.InputMethod.xml
    org.fcitx.Fcitx.InputMethod
)

QT4_ADD_DBUS_INTERFACE(kcm_SRCS
    org.fcitx.Fcitx.Keyboard.xml
    org.fcitx.Fcitx.Keyboard
)

kde4_add_ui_files(kcm_SRCS Module.ui FcitxConfigPage.ui FcitxSkinPage.ui FcitxIMPage.ui)

kde4_add_plugin(kcm_fcitx ${kcm_SRCS})

target_link_libraries(kcm_fcitx
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDE4_KNEWSTUFF3_LIBS}
    ${FCITX4_FCITX_UTILS_LIBRARIES}
    ${FCITX4_FCITX_CONFIG_LIBRARIES}
    ${FCITX4_FCITX_LIBRARIES}
    ${LIBINTL_LIBRARY}
)

install(TARGETS kcm_fcitx DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_fcitx.desktop DESTINATION ${SERVICES_INSTALL_DIR})

message("GHNS DIR : ${CONFIG_INSTALL_DIR}")

install(FILES fcitx-skin.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
