cmake_minimum_required(VERSION 3.28)
project(DeerFont VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEERFONT_TEST "Enables The Tests For The Font Library" ON)

add_library(${PROJECT_NAME}
        Source/Includes/TrueType/TTF.hpp
        Source/Includes/TrueType/TTF_Cmap.hpp
        Source/Includes/TrueType/TTF_Glyf.hpp
        Source/Includes/TrueType/TTF_Head.hpp
        Source/Includes/TrueType/TTF_Hhea.hpp
        Source/Includes/TrueType/TTF_Hmtx.hpp
        Source/Includes/TrueType/TTF_Loca.hpp
        Source/Includes/TrueType/TTF_Maxp.hpp
        Source/Includes/Helpers.hpp
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET deerfont_module TYPE CXX_MODULES FILES
        Source/Interface/DeerFont.ixx
        Source/Interface/Font.ixx
        Source/Interface/TTF.ixx
)
target_include_directories(${PROJECT_NAME} PRIVATE Source/Includes)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-DBALBINO_DEBUG>)
include(${CMAKE_SOURCE_DIR}/CMake/SetupFawnAlgebra.cmake)
setup_fawnalgebra(${PROJECT_NAME})

if (DEERFONT_TEST)
    add_subdirectory(tests)
endif ()