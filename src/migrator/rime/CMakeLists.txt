add_library(rimemigrator STATIC rimemigrator.cpp)
target_link_libraries(rimemigrator migrator)
target_compile_definitions(rimemigrator PRIVATE "-DQT_STATICPLUGIN")
set_target_properties(rimemigrator PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

