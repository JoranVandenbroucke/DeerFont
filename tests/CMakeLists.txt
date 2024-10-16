set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}_test
        ttf.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/SetupCatch.cmake)
setup_catch(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME}_test PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX /MP $<$<CONFIG:Debug>:${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT} ${DEFAULT_CXX_EXCEPTION_HANDLING}>>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
        PUBLIC
        $<$<CXX_COMPILER_ID:GNU>:-fmodules-ts>
        #todo --embed-dir=${CMAKE_SOURCE_DIR}/Embedded/
        $<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++ -fembed-bitcode -Wno-deprecated -Wno-deprecated-declarations>
)

target_link_libraries(${PROJECT_NAME}_test PRIVATE
        ${PROJECT_NAME}::${PROJECT_NAME}
)

add_custom_command(TARGET ${PROJECT_NAME}_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts
        ${CMAKE_CURRENT_BINARY_DIR}/fonts
)
