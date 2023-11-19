add_library(configwidgetslib STATIC
    configwidget.cpp
    addonselector.cpp
    categoryhelper.cpp
    erroroverlay.cpp
    fontbutton.cpp
    keylistwidget.cpp
    layoutselector.cpp
    listoptionwidget.cpp
    optionwidget.cpp
    varianthelper.cpp
    verticalscrollarea.cpp
    impage.cpp
)
set_target_properties(configwidgetslib PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_include_directories(configwidgetslib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(configwidgetslib
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Widgets
    #Qt${QT_MAJOR_VERSION}::X11Extras
    KF${QT_MAJOR_VERSION}::WidgetsAddons
    KF${QT_MAJOR_VERSION}::ItemViews
    Fcitx5::Utils
    Fcitx5Qt${QT_MAJOR_VERSION}::DBusAddons
    Fcitx5Qt${QT_MAJOR_VERSION}::WidgetsAddons
    configlib
    layoutlib)
