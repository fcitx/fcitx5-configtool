add_library(kcm_fcitx5 MODULE
    main.cpp
)

target_link_libraries(kcm_fcitx5
  KF5::CoreAddons
  KF5::I18n
  KF5::QuickAddons
  XKBCommon::XKBCommon
  configlib
)

file(GLOB_RECURSE inFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/package/*")
foreach(infileName ${inFiles})
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${infileName}"
        "${CMAKE_CURRENT_BINARY_DIR}/${infileName}" @ONLY
    )
endforeach()


fcitx5_translate_desktop_file(kcm_fcitx5.desktop.in
  kcm_fcitx5.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/org.fcitx.fcitx5.kcm)
fcitx5_translate_desktop_file(package/metadata.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/package/metadata.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/org.fcitx.fcitx5.kcm)
kcoreaddons_desktop_to_json(kcm_fcitx5 "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop" DEFAULT_SERVICE_TYPE)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop"
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_fcitx5 DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
# This is a hack against kpackage_install_package
# 1. kpackage_install_package only accept relative path to ${CMAKE_CURRENT_SOURCE_DIR}
# 2. It need metadata.desktop to run properly, but we only have untranslated file at that time.
#    So we just delete the file during the cmake.
file(RELATIVE_PATH RELATIVE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
kpackage_install_package(${RELATIVE_BINARY_DIR}/package org.fcitx.fcitx5.kcm kcms)
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/package/metadata.desktop)
