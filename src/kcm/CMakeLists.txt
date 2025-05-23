kcmutils_add_qml_kcm(kcm_fcitx5 SOURCES main.cpp)
target_link_libraries(kcm_fcitx5 KF${QT_MAJOR_VERSION}::KCMUtilsQuick)

target_link_libraries(kcm_fcitx5
  Qt${QT_MAJOR_VERSION}::Quick
  KF${QT_MAJOR_VERSION}::CoreAddons
  KF${QT_MAJOR_VERSION}::I18n
  XKBCommon::XKBCommon
  configlib
)
