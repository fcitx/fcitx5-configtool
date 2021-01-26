set(configtool_SRCS
  main.cpp
  mainwindow.cpp
  )

add_executable(fcitx5-config-qt ${configtool_SRCS})
set_target_properties(fcitx5-config-qt
    PROPERTIES
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h")

target_link_libraries(fcitx5-config-qt
    Qt5::Widgets configwidgetslib
)

install(TARGETS fcitx5-config-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
fcitx5_translate_desktop_file(org.fcitx.fcitx5-config-qt.desktop.in
  org.fcitx.fcitx5-config-qt.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/fcitx5-configtool)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.fcitx5-config-qt.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
