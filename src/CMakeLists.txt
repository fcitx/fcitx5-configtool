include_directories(${PROJECT_SOURCE_DIR}/layout)
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_fcitx\")

set(kcm_SRCS
  main.cpp
  module.cpp
  addonselector.cpp
  configwidget.cpp
  global.cpp
  subconfigparser.cpp
  subconfigpattern.cpp
  subconfig.cpp
  subconfigwidget.cpp
  skinpage.cpp
  impage.cpp
  uipage.cpp
  imconfigdialog.cpp
  dummyconfig.cpp
  verticalscrollarea.cpp
  fontbutton.cpp
  configpage.cpp
  plugindialog.cpp
  erroroverlay.cpp
#   ../layout/keyboardlayoutwidget.cpp
  )

set(kcm_UIS module.ui skinpage.ui impage.ui fontbutton.ui configpage.ui)
fcitx_translate_add_sources(${kcm_SRCS} ${kcm_UIS} kcm_fcitx.desktop.in)

ki18n_wrap_ui(kcm_SRCS ${kcm_UIS})

add_library(kcm_fcitx MODULE ${kcm_SRCS})

target_link_libraries(kcm_fcitx
  KF5::CoreAddons
  KF5::WidgetsAddons
  KF5::KCMUtils
  KF5::ItemViews
  KF5::I18n
  KF5::KIOCore
  KF5::KIOWidgets
  KF5::NewStuff
  FcitxQt5::WidgetsAddons
  FcitxQt5::DBusAddons
  ${FCITX4_FCITX_UTILS_LIBRARIES}
  ${FCITX4_FCITX_CONFIG_LIBRARIES}
  ${FCITX4_FCITX_LIBRARIES}
  ${LIBINTL_LIBRARY}
  )

install(TARGETS kcm_fcitx DESTINATION ${CMAKE_INSTALL_PLUGINDIR})
fcitx_translate_add_apply_source(
  kcm_fcitx.desktop.in
  "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx.desktop")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx.desktop"
  DESTINATION ${CMAKE_INSTALL_KSERVICES5DIR})

message("GHNS DIR : ${CONFIG_INSTALL_DIR}")

install(FILES fcitx-skin.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

if(ENABLE_TEST)
  add_subdirectory(test)
endif(ENABLE_TEST)
