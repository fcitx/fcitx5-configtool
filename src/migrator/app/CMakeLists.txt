add_executable(fcitx5-migrator main.cpp mainwindow.cpp welcomepage.cpp taskpage.cpp progresspage.cpp)
set_target_properties(fcitx5-migrator PROPERTIES
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h")
target_link_libraries(fcitx5-migrator pinyinmigrator skkmigrator rimemigrator kkcmigrator globalconfigmigrator tablemigrator Qt5::Widgets KF5::WidgetsAddons configlib Fcitx5Migrator Fcitx5Qt5::WidgetsAddons Fcitx5Qt5::DBusAddons)
install(TARGETS fcitx5-migrator DESTINATION ${CMAKE_INSTALL_BINDIR})
