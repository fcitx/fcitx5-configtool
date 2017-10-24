include_directories(
  ${X11_Xkb_INCLUDE_PATH}
  ${X11_Xkblib_INCLUDE_PATH}
  )

set(kbd_layout_viewer_SOURCES
  main.cpp
  keyboardlayoutwidget.cpp
  )

add_executable(kbd-layout-viewer ${kbd_layout_viewer_SOURCES})

target_link_libraries(kbd-layout-viewer Fcitx5::Utils Qt5::Widgets KF5::I18n Qt5::X11Extras ${X11_X11_LIB} ${XKBFILE_LIBRARIES} ${X11_Xkblib_LIB})

install(TARGETS kbd-layout-viewer DESTINATION ${KDE_INSTALL_BINDIR})
fcitx5_translate_desktop_file(
  kbd-layout-viewer.desktop.in
  kbd-layout-viewer.desktop)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kbd-layout-viewer.desktop"
  DESTINATION ${KDE_INSTALL_APPDIR})
