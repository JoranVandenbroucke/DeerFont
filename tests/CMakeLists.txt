add_executable(${PROJECT_NAME}_test
        ttf.cpp
)

set_target_properties(${PROJECT_NAME}_test PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME}_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_definitions(${PROJECT_NAME}_test PRIVATE $<$<CONFIG:Debug>:-DBALBINO_DEBUG>)
target_compile_features(${PROJECT_NAME}_test PRIVATE cxx_std_23)

include(compiler_select_flags)
include(compiler_select_sdl)
include(compiler_select_linker)
include(setup_catch)
set_compile_flags(${PROJECT_NAME})
select_best_stl(${PROJECT_NAME}_test)
select_best_linker(${PROJECT_NAME}_test)
setup_catch(${PROJECT_NAME}_test)

target_link_libraries(${PROJECT_NAME}_test PRIVATE
        ${PROJECT_NAME}::${PROJECT_NAME}
)

add_custom_command(TARGET ${PROJECT_NAME}_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts
        ${CMAKE_CURRENT_BINARY_DIR}/fonts
)
