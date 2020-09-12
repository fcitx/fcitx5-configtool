add_library(kkcmigrator STATIC kkcmigrator.cpp)
target_link_libraries(kkcmigrator migrator)
target_compile_definitions(kkcmigrator PRIVATE "-DQT_STATICPLUGIN")
set_target_properties(kkcmigrator PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

