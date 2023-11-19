add_library(configlib STATIC
    iso639.cpp
    dbusprovider.cpp
    model.cpp
    addonmodel.cpp
    imconfig.cpp
    logging.cpp
    font.cpp
    layoutmodel.cpp
    layoutprovider.cpp
)
set_target_properties(configlib PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_include_directories(configlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(configlib Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Gui Fcitx5Qt${QT_MAJOR_VERSION}::DBusAddons Fcitx5::Core Fcitx5::Utils)

