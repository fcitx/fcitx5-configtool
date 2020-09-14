ecm_setup_version(PROJECT VARIABLE_PREFIX Fcitx5Migrator
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/migrator_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/Fcitx5MigratorVersion.cmake"
                        SOVERSION 0)

add_library(Fcitx5Migrator SHARED
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
target_link_libraries(Fcitx5Migrator Qt5::Core Qt5::Concurrent Qt5::DBus Fcitx5::Config Fcitx5::Utils Fcitx5Qt5::DBusAddons )
set_target_properties(Fcitx5Migrator PROPERTIES
    VERSION ${Fcitx5Migrator_VERSION}
    SOVERSION ${Fcitx5Migrator_SOVERSION}
    EXPORT_NAME Migrator
)

target_include_directories(Fcitx5Migrator PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}>")

generate_export_header(Fcitx5Migrator BASE_NAME Fcitx5Migrator)

install(TARGETS Fcitx5Migrator LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
