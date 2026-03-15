cmake_minimum_required(VERSION 3.28)
project(DeerFont VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEERFONT_TEST "Enables The Tests For The Font Library" ON)

add_library(${PROJECT_NAME}
        source/Includes/TrueType/TTF.hpp
        source/Includes/TrueType/TTF_Cmap.hpp
        source/Includes/TrueType/TTF_Glyf.hpp
        source/Includes/TrueType/TTF_Head.hpp
        source/Includes/TrueType/TTF_Hhea.hpp
        source/Includes/TrueType/TTF_Hmtx.hpp
        source/Includes/TrueType/TTF_Loca.hpp
        source/Includes/TrueType/TTF_Maxp.hpp
        source/Includes/Helpers.hpp
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET deerfont_module TYPE CXX_MODULES FILES
        source/Interface/DeerFont.ixx
        source/Interface/Font.ixx
        source/Interface/TTF.ixx
)
target_include_directories(${PROJECT_NAME} PRIVATE source/Includes)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-DBALBINO_DEBUG>)
include(setup_fawnalgebra)
setup_fawnalgebra(${PROJECT_NAME})

if (DEERFONT_TEST)
    add_subdirectory(tests)
endif ()
