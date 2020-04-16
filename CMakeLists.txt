cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(kcm-fcitx VERSION 0.5.99)

set(QT_MIN_VERSION "5.3.0")
find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)
option(ENABLE_KCM "Enable KDE Config Module" On)
option(ENABLE_CONFIG_QT "Enable fcitx5-config-qt" On)
option(ENABLE_TEST "Enable test" Off)
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_fcitx5\")
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"kcm_fcitx5\")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets X11Extras)

if (ENABLE_CONFIG_QT)
    find_package(KF5 REQUIRED COMPONENTS
        WidgetsAddons
        ItemViews
    )
    find_package(Fcitx5Qt5WidgetsAddons REQUIRED)
endif()

if (ENABLE_KCM)
    find_package(KF5CoreAddons REQUIRED)
    find_package(KF5I18n REQUIRED)
    find_package(KF5Package REQUIRED)
    find_package(KF5Declarative REQUIRED)
    find_package(KF5Kirigami2 REQUIRED)
    find_package(XKBCommon REQUIRED COMPONENTS XKBCommon)
endif()

find_package(Fcitx5Utils REQUIRED)
find_package(Fcitx5Qt5DBusAddons REQUIRED)
find_package(IsoCodes REQUIRED)

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
