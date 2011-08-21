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

option(ENABLE_DEBUG "Debug On" Off)

find_package(Qt4 4.7.0 REQUIRED)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(PkgConfig REQUIRED)
FIND_PACKAGE(Gettext REQUIRED)
find_package(Fcitx4 REQUIRED)
FIND_PROGRAM(INTLTOOL_EXTRACT intltool-extract)
FIND_PROGRAM(INTLTOOL_UPDATE intltool-update)
FIND_PROGRAM(INTLTOOL_MERGE intltool-merge)

set(package ${FCITX4_PACKAGE_NAME})
set(datadir ${CMAKE_INSTALL_PREFIX}/share)
set(localedir ${CMAKE_INSTALL_PREFIX}/share/locale)
set(exec_prefix "${CMAKE_INSTALL_PREFIX}/bin")

if (ENABLE_DEBUG)
    set(CMAKE_CXX_FLAGS "-g -O0")
endif (ENABLE_DEBUG)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(po)
