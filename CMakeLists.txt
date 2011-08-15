cmake_minimum_required (VERSION 2.6)

project(fcitx-configtool)
set(MAJOR_VERSION "0")
set(MINOR_VERSION "1")
set(PATCH_VERSION "0")
set(SUFFIX_VERSION "")

set(VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
if(NOT SUFFIX_VERSION)
    set(VERSION_STRING_FULL "${VERSION_STRING}")
else(NOT SUFFIX_VERSION)
    set(VERSION_STRING_FULL "${VERSION_STRING}-${SUFFIX_VERSION}")
endif(NOT SUFFIX_VERSION)

include(FindPkgConfig)
FIND_PACKAGE(Gettext REQUIRED)
FIND_PROGRAM(INTLTOOL_EXTRACT intltool-extract)
FIND_PROGRAM(INTLTOOL_UPDATE intltool-update)
FIND_PROGRAM(INTLTOOL_MERGE intltool-merge)

PKG_CHECK_MODULES(FCITX_CONFIG REQUIRED "fcitx-config >= 1.0" )
PKG_CHECK_MODULES(FCITX REQUIRED "fcitx" )

set(datadir ${CMAKE_INSTALL_PREFIX}/share)
set(localedir ${CMAKE_INSTALL_PREFIX}/share/locale)
set(exec_prefix "${CMAKE_INSTALL_PREFIX}/bin")

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(po)
