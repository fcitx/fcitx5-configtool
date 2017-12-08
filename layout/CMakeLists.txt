add_library(layoutlib STATIC keyboardlayoutwidget.cpp)
set_target_properties(layoutlib PROPERTIES COMPILE_FLAGS "-fPIC")
target_link_libraries(layoutlib PUBLIC Qt5::Widgets PRIVATE Qt5::X11Extras Fcitx5::Utils ${X11_X11_LIB} ${XKBFILE_LIBRARIES} ${X11_Xkblib_LIB})

target_include_directories(layoutlib PRIVATE
  ${X11_Xkb_INCLUDE_PATH}
  ${X11_Xkblib_INCLUDE_PATH}
  )

set(kbd_layout_viewer_SOURCES
  main.cpp
  )

add_executable(kbd-layout-viewer5 ${kbd_layout_viewer_SOURCES})

target_link_libraries(kbd-layout-viewer5 Fcitx5::Utils Qt5::Widgets Qt5::X11Extras KF5::I18n layoutlib)

install(TARGETS kbd-layout-viewer5 DESTINATION ${KDE_INSTALL_BINDIR})
fcitx5_translate_desktop_file(
  kbd-layout-viewer5.desktop.in
  kbd-layout-viewer5.desktop)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kbd-layout-viewer5.desktop"
  DESTINATION ${KDE_INSTALL_APPDIR})
