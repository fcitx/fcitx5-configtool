add_executable(fcitx5-plasma-theme-generator main.cpp)

target_link_libraries(fcitx5-plasma-theme-generator
    KF${QT_MAJOR_VERSION}::Plasma
    KF${QT_MAJOR_VERSION}::IconThemes
    KF${QT_MAJOR_VERSION}::I18n
    Fcitx5::Utils
    Fcitx5::Config)

if (QT_MAJOR_VERSION STREQUAL 6)
target_link_libraries(fcitx5-plasma-theme-generator
    KF${QT_MAJOR_VERSION}::Svg)
endif()

install(TARGETS fcitx5-plasma-theme-generator DESTINATION ${CMAKE_INSTALL_BINDIR})
