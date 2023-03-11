cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "In-source builds are disabled.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake will now create CMakeCache.txt and CMakeFiles/*.
          You must delete them, or cmake will refuse to work.")
endif()

project(fcitx5-configtool VERSION 5.0.17)

set(QT_MIN_VERSION "5.12.0")
find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)
include(ECMSetupVersion)
include(GenerateExportHeader)
option(ENABLE_KCM "Enable KDE Config Module" On)
option(ENABLE_CONFIG_QT "Enable fcitx5-config-qt" On)
option(ENABLE_TEST "Enable test" Off)
add_definitions(-DTRANSLATION_DOMAIN=\"org.fcitx.fcitx5.kcm\")
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-configtool\")
add_definitions(-DQT_NO_KEYWORDS)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets X11Extras Concurrent)

if (ENABLE_CONFIG_QT)
    find_package(KF5ItemViews REQUIRED)
endif()

if (ENABLE_KCM)
    find_package(Qt5QuickControls2 REQUIRED)
    find_package(KF5CoreAddons REQUIRED)
    find_package(KF5I18n REQUIRED)
    find_package(KF5Package REQUIRED)
    find_package(KF5Declarative REQUIRED)
    find_package(KF5IconThemes REQUIRED)
    find_package(KF5Kirigami2 5.68 REQUIRED)
    find_package(XKBCommon REQUIRED COMPONENTS XKBCommon)
    find_package(KF5Plasma REQUIRED)

    if (KF5Kirigami2_VERSION VERSION_LESS 5.76)
        set(DISABLE_UNDER_KIRIGAMI2_5_76 "//Needs Kirigami2 5.76 ")
    else()
        set(DISABLE_UNDER_KIRIGAMI2_5_76 "")
    endif()
endif()

find_package(Fcitx5Core 5.0.4 REQUIRED)
find_package(Fcitx5Config REQUIRED)
find_package(Fcitx5Utils REQUIRED)
find_package(Fcitx5Qt5DBusAddons REQUIRED)
find_package(Fcitx5Qt5WidgetsAddons 5.0.2 REQUIRED)
find_package(Qt5Svg)
set_package_properties(Qt5Svg PROPERTIES
    DESCRIPTION "May be used for show icons"
    TYPE RUNTIME)

get_target_property(FCITX5_QT5_GUI_WRAPPER Fcitx5Qt5::gui-wrapper IMPORTED_LOCATION)

find_package(KF5WidgetsAddons REQUIRED)
find_package(IsoCodes REQUIRED)
kde_enable_exceptions()

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

find_package(PkgConfig REQUIRED)
find_package(Gettext REQUIRED)
find_package(X11 REQUIRED)
find_package(XKeyboardConfig REQUIRED)
pkg_check_modules(XkbFile "xkbfile" REQUIRED IMPORTED_TARGET)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(layout)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
