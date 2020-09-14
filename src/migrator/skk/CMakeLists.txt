add_library(skkmigrator STATIC skkmigrator.cpp)
target_link_libraries(skkmigrator Fcitx5Migrator)
target_compile_definitions(skkmigrator PRIVATE "-DQT_STATICPLUGIN")
set_target_properties(skkmigrator PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

