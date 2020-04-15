add_library(kcm_fcitx5 MODULE
    main.cpp
    qtkeytrans.cpp
)

target_link_libraries(kcm_fcitx5
  KF5::CoreAddons
  KF5::I18n
  KF5::QuickAddons
  XKBCommon::XKBCommon
  configlib
)

fcitx5_translate_desktop_file(kcm_fcitx5.desktop.in
  kcm_fcitx5.desktop)
kcoreaddons_desktop_to_json(kcm_fcitx5 "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop" DEFAULT_SERVICE_TYPE)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop"
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_fcitx5 DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_fcitx5 kcms)
