ecm_setup_version(PROJECT VARIABLE_PREFIX FCITX5QT4DBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/migrator_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5MigratorVersion.cmake"
                        SOVERSION 1)

add_library(migrator SHARED
    migratorfactoryplugin.cpp
    migrator.cpp
    migratorfactory.cpp
    pipeline.cpp
    pipelinejob.cpp
    processrunner.cpp
    dbuscaller.cpp
    dbuswatcher.cpp
    renamefile.cpp
    copydirectory.cpp
    callbackrunner.cpp
    log.cpp
    configmigrator.cpp
    )
target_link_libraries(migrator Qt5::Core Qt5::Concurrent Qt5::DBus Fcitx5::Config Fcitx5::Utils Fcitx5Qt5::DBusAddons )

target_include_directories(migrator PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}>")

generate_export_header(migrator BASE_NAME Fcitx5Migrator)
