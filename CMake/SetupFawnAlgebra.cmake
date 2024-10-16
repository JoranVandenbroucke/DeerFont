function(setup_fawnalgebra target_name)
    # Check if FawnAlgebra is already a target
    if (NOT TARGET FawnAlgebra::FawnAlgebra)
        # Try to find FawnAlgebra package
        find_package(FawnAlgebra QUIET PATHS "${CMAKE_SOURCE_DIR}/3rdParty/FawnAlgebra")

        # If FawnAlgebra is not found, attempt to add it from source or fetch it
        if (NOT FawnAlgebra_FOUND)
            if (EXISTS "${CMAKE_SOURCE_DIR}/3rdParty/FawnAlgebra/CMakeLists.txt")
                add_subdirectory("${CMAKE_SOURCE_DIR}/3rdParty/FawnAlgebra" FawnAlgebra EXCLUDE_FROM_ALL)
            elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/FawnAlgebra/CMakeLists.txt")
                add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/FawnAlgebra" FawnAlgebra EXCLUDE_FROM_ALL)
            else ()
                include(FetchContent)
                FetchContent_Declare(
                        FawnAlgebra
                        GIT_REPOSITORY https://github.com/JoranVandenbroucke/FawnAlgebra.git
                        GIT_TAG main  # Replace with a specific tag or commit hash if needed
                        GIT_SHALLOW TRUE
                        GIT_PROGRESS TRUE
                )
                FetchContent_MakeAvailable(FawnAlgebra)
                set_property(DIRECTORY "${CMAKE_BINARY_DIR}/_deps/FawnAlgebra-src" PROPERTY EXCLUDE_FROM_ALL TRUE)
            endif ()
        endif ()
    endif ()

    # Add FawnAlgebra as a dependency and link it to the target
    add_dependencies(${target_name} FawnAlgebra::FawnAlgebra)
    target_include_directories(${target_name} PUBLIC ${FawnAlgebra_INCLUDE_DIRS})
    target_link_libraries(${target_name} PUBLIC FawnAlgebra::FawnAlgebra)
endfunction()
