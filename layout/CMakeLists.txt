include_directories(
  ${X11_Xkb_INCLUDE_PATH}
  ${X11_Xkblib_INCLUDE_PATH}
  )

set(kbd_layout_viewer_SOURCES
  main.cpp
  keyboardlayoutwidget.cpp
  )
fcitx_translate_add_sources(${kbd_layout_viewer_SOURCES}
  kbd-layout-viewer.desktop.in)

kde4_add_executable(kbd-layout-viewer ${kbd_layout_viewer_SOURCES})

target_link_libraries(kbd-layout-viewer ${FCITX4_FCITX_CONFIG_LIBRARIES} ${KDE4_KDEUI_LIBRARY} ${X11_X11_LIB} ${XKBFILE_LIBRARIES})

install(TARGETS kbd-layout-viewer DESTINATION ${BIN_INSTALL_DIR})
fcitx_translate_add_apply_source(
  kbd-layout-viewer.desktop.in
  "${CMAKE_CURRENT_BINARY_DIR}/kbd-layout-viewer.desktop")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kbd-layout-viewer.desktop"
  DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
