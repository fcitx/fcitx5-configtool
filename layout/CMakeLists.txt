include_directories(
    ${X11_Xkb_INCLUDE_PATH}
    ${X11_Xkblib_INCLUDE_PATH}
    )

set(kbd_layout_viewer_SOURCES main.cpp
    KeyboardLayoutWidget.cpp
    )

kde4_add_executable(kbd-layout-viewer ${kbd_layout_viewer_SOURCES})

target_link_libraries(kbd-layout-viewer ${FCITX4_FCITX_CONFIG_LIBRARIES} ${KDE4_KDEUI_LIBRARY} ${X11_X11_LIB} ${XKBFILE_LIBRARIES})

install(TARGETS kbd-layout-viewer DESTINATION ${BIN_INSTALL_DIR})
install(FILES kbd-layout-viewer.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})