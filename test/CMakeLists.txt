include_directories(../layout)

add_executable(layout_test layouttest.cpp)

target_link_libraries(layout_test
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_X11_X11_LIBRARY}
    layout
)
