add_executable(fcitx5-migrator main.cpp mainwindow.cpp welcomepage.cpp taskpage.cpp progresspage.cpp)
set_target_properties(fcitx5-migrator PROPERTIES
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h")
target_link_libraries(fcitx5-migrator
   pinyinmigrator
   skkmigrator
   rimemigrator
   kkcmigrator
   globalconfigmigrator
   tablemigrator
   Qt${QT_MAJOR_VERSION}::Widgets
   KF${QT_MAJOR_VERSION}::WidgetsAddons
   configlib
   Fcitx5Migrator
   Fcitx5Qt${QT_MAJOR_VERSION}::WidgetsAddons
   Fcitx5Qt${QT_MAJOR_VERSION}::DBusAddons)
install(TARGETS fcitx5-migrator DESTINATION ${CMAKE_INSTALL_BINDIR})

fcitx5_translate_desktop_file(org.fcitx.fcitx5-migrator.desktop.in
  org.fcitx.fcitx5-migrator.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/fcitx5-configtool)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.fcitx5-migrator.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
