include_directories(${PROJECT_SOURCE_DIR}/layout)

set(kcm_SRCS
  main.cpp
  module.cpp
  addonselector.cpp
  configwidget.cpp
  configdescmanager.cpp
  subconfigparser.cpp
  subconfigpattern.cpp
  subconfig.cpp
  subconfigwidget.cpp
  skinpage.cpp
  impage.cpp
  im.cpp
  uipage.cpp
  layout.cpp
  imconfigdialog.cpp
  qtkeytrans.cpp
  inputmethodproxy.cpp
  keyboardproxy.cpp
  dummyconfig.cpp
  verticalscrollarea.cpp
  fontbutton.cpp
  ../layout/keyboardlayoutwidget.cpp
  )

set(kcm_UIS module.ui skinpage.ui impage.ui fontbutton.ui)
fcitx_translate_add_sources(${kcm_SRCS} ${kcm_UIS} kcm_fcitx.desktop.in)

kde4_add_ui_files(kcm_SRCS ${kcm_UIS})

kde4_add_plugin(kcm_fcitx ${kcm_SRCS})

target_link_libraries(kcm_fcitx
  ${KDE4_KDEUI_LIBRARY}
  ${KDE4_KIO_LIBS}
  ${KDE4_KNEWSTUFF3_LIBS}
  ${FCITX4_FCITX_UTILS_LIBRARIES}
  ${FCITX4_FCITX_CONFIG_LIBRARIES}
  ${FCITX4_FCITX_LIBRARIES}
  ${LIBINTL_LIBRARY}
  ${XKBFILE_LIBRARIES}
  ${X11_X11_LIB}
  )

install(TARGETS kcm_fcitx DESTINATION ${PLUGIN_INSTALL_DIR})
fcitx_translate_add_apply_source(
  kcm_fcitx.desktop.in
  "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx.desktop")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kcm_fcitx.desktop"
  DESTINATION ${SERVICES_INSTALL_DIR})

message("GHNS DIR : ${CONFIG_INSTALL_DIR}")

install(FILES fcitx-skin.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

if(ENABLE_TEST)
  add_subdirectory(test)
endif()
