set(kcm_SRCS
  main.cpp
  module.cpp
  )

add_library(kcm_fcitx5 MODULE ${kcm_SRCS})
set_target_properties(kcm_fcitx5 PROPERTIES AUTOMOC TRUE AUTOUIC TRUE)

target_link_libraries(kcm_fcitx5
  KF5::CoreAddons
  KF5::ConfigWidgets
  KF5::I18n
  configlib
  )

install(TARGETS kcm_fcitx5 DESTINATION ${KDE_INSTALL_PLUGINDIR})
fcitx5_translate_desktop_file(kcm_fcitx5.desktop.in
  kcm_fcitx5.desktop)
kcoreaddons_desktop_to_json(kcm_fcitx5 "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop" SERVICE_TYPES kcmodule.desktop)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop"
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
