add_library(tablemigrator STATIC tablemigrator.cpp)
target_link_libraries(tablemigrator Fcitx5Migrator)
target_compile_definitions(tablemigrator PRIVATE "-DQT_STATICPLUGIN")
set_target_properties(tablemigrator PROPERTIES POSITION_INDEPENDENT_CODE TRUE)


