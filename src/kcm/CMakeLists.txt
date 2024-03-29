if (QT_MAJOR_VERSION STREQUAL 5)
    add_library(kcm_fcitx5 MODULE
        main.cpp
    )
    target_link_libraries(kcm_fcitx5 KF${QT_MAJOR_VERSION}::QuickAddons KF${QT_MAJOR_VERSION}::Declarative)

    file(GLOB_RECURSE inFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/package/*")
    foreach(infileName ${inFiles})
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${infileName}"
        "${CMAKE_CURRENT_BINARY_DIR}/${infileName}" @ONLY
    )
    endforeach()

    # This is a hack against kpackage_install_package
    # 1. kpackage_install_package only accept relative path to ${CMAKE_CURRENT_SOURCE_DIR}
    # 2. It need metadata.desktop to run properly, but we only have untranslated file at that time.
    #    So we just delete the file during the cmake.
    file(RELATIVE_PATH RELATIVE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    if (KF5Declarative_VERSION VERSION_GREATER_EQUAL 5.104.0)
      install(TARGETS kcm_fcitx5 DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)
      kpackage_install_package(${RELATIVE_BINARY_DIR}/package kcm_fcitx5 kcms)
      fcitx5_translate_desktop_file(kcm_fcitx5.desktop.in
        kcm_fcitx5.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/kcm_fcitx5)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop"
        DESTINATION ${KDE_INSTALL_APPDIR})
    else()
      install(TARGETS kcm_fcitx5 DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
      fcitx5_translate_desktop_file(kcm_fcitx5_kservice.desktop.in
        kcm_fcitx5_kservice.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/kcm_fcitx5)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5_kservice.desktop"
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME kcm_fcitx5.desktop)
    endif()
    
elseif(QT_MAJOR_VERSION STREQUAL 6)
    kcmutils_add_qml_kcm(kcm_fcitx5 SOURCES main.cpp)
    target_link_libraries(kcm_fcitx5 KF${QT_MAJOR_VERSION}::KCMUtilsQuick)
endif()

target_link_libraries(kcm_fcitx5
  Qt${QT_MAJOR_VERSION}::Quick
  KF${QT_MAJOR_VERSION}::CoreAddons
  KF${QT_MAJOR_VERSION}::I18n
  XKBCommon::XKBCommon
  configlib
)
