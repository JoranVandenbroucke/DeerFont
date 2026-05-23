cmake_minimum_required(VERSION 4.3.0)
set(CMAKE_CXX_COMPILER_IMPORT_STD ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
project(DeerFont VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_MODULE_STD 1)

option(DEERFONT_TEST "Enables The Tests For The Font Library" OFF)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET deerfont_module TYPE CXX_MODULES
        BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Interface
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Interface/DeerFont.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Interface/Font.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Interface/TTF.ixx

        PRIVATE
        FILE_SET deerfont_include TYPE HEADERS
        BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF_Cmap.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF_Glyf.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF_Head.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF_Hhea.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF_Hmtx.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF_Loca.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/TrueType/TTF_Maxp.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Includes/Helpers.hpp
)
target_include_directories(${PROJECT_NAME} PRIVATE source/Includes)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-DBALBINO_DEBUG>)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS        OFF
    CXX_SCAN_FOR_MODULES  ON
)

include(compiler_select_flags)
include(compiler_select_sdl)
include(compiler_select_linker)
include(setup_fawnalgebra)
set_compile_flags(${PROJECT_NAME})
select_best_stl(${PROJECT_NAME})
select_best_linker(${PROJECT_NAME})
setup_fawnalgebra(${PROJECT_NAME})

if (DEERFONT_TEST)
    add_subdirectory(tests)
endif ()
