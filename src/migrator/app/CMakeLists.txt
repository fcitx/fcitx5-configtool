add_executable(fcitx5-migrator main.cpp mainwindow.cpp welcomepage.cpp taskpage.cpp progresspage.cpp)
set_target_properties(fcitx5-migrator PROPERTIES AUTOMOC TRUE AUTOUIC TRUE)
target_link_libraries(fcitx5-migrator pinyinmigrator skkmigrator rimemigrator kkcmigrator globalconfigmigrator tablemigrator Qt5::Widgets KF5::WidgetsAddons configlib migrator Fcitx5Qt5::DBusAddons)

