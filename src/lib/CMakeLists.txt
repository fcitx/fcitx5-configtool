add_library(configlib STATIC
    iso639.cpp
    configwidget.cpp
    dbusprovider.cpp
    addonselector.cpp
    categoryhelper.cpp
    erroroverlay.cpp
    fontbutton.cpp
    keylistwidget.cpp
    layoutselector.cpp
    listoptionwidget.cpp
    logging.cpp
    model.cpp
    optionwidget.cpp
    varianthelper.cpp
    verticalscrollarea.cpp
    impage.cpp
)
set_target_properties(configlib PROPERTIES
    COMPILE_FLAGS "-fPIC"
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_include_directories(configlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(configlib Qt5::Core Qt5::Widgets Qt5::X11Extras KF5::WidgetsAddons KF5::ItemViews Fcitx5::Utils Fcitx5Qt5::DBusAddons Fcitx5Qt5::WidgetsAddons layoutlib)
