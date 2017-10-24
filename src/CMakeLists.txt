include_directories(${PROJECT_SOURCE_DIR}/layout)
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_fcitx5\")

set(kcm_SRCS
  main.cpp
  module.cpp
  erroroverlay.cpp
  impage.cpp
  model.cpp
  layoutselector.cpp
  iso639.cpp
  ../layout/keyboardlayoutwidget.cpp
  )

set(kcm_UIS module.ui impage.ui fontbutton.ui erroroverlay.ui layoutselector.ui)

ki18n_wrap_ui(kcm_SRCS ${kcm_UIS})

add_library(kcm_fcitx5 MODULE ${kcm_SRCS})

target_link_libraries(kcm_fcitx5
  Qt5::Widgets
  Qt5::X11Extras
  KF5::CoreAddons
  KF5::WidgetsAddons
  KF5::ConfigWidgets
  KF5::ItemViews
  KF5::I18n
  KF5::KIOCore
  KF5::KIOWidgets
  Fcitx5::Utils
  Fcitx5Qt5::DBusAddons
  ${X11_X11_LIB} ${XKBFILE_LIBRARIES} ${X11_Xkblib_LIB}
  )

install(TARGETS kcm_fcitx5 DESTINATION ${KDE_INSTALL_PLUGINDIR})
fcitx5_translate_desktop_file(kcm_fcitx5.desktop.in
  kcm_fcitx5.desktop)
kcoreaddons_desktop_to_json(kcm_fcitx5 "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop" SERVICE_TYPES kcmodule.desktop)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx5.desktop"
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
