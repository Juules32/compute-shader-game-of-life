cmake_minimum_required(VERSION 3.10)
project(compute-shader-game-of-life)

# C++ settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Apple specific
if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COREVIDEO_LIBRARY CoreVideo)

    set(APPLE_LIBRARIES
        ${OPENGL_LIBRARY}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()

# For zed language server functionality
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include paths
set(LIBRARIES_SOURCE_PATH ${CMAKE_SOURCE_DIR}/libraries)

# Where to look for include directories
include_directories(
        ${LIBRARIES_SOURCE_PATH}/glad/include
        ${LIBRARIES_SOURCE_PATH}/glfw/include
        ${LIBRARIES_SOURCE_PATH}/imgui/include
        ${LIBRARIES_SOURCE_PATH}/glm/include
        ${LIBRARIES_SOURCE_PATH}/stb/include
        ${LIBRARIES_SOURCE_PATH}/assimp/include
        ${LIBRARIES_SOURCE_PATH}/assimp
        ${LIBRARIES_SOURCE_PATH}/itugl/include
)

# Libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/libraries)

# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HDR_FILES CONFIGURE_DEPENDS "src/*.h")
file(GLOB_RECURSE SHADERS CONFIGURE_DEPENDS "src/shaders/*.*")

# Copy shaders incrementally when changed
foreach(SHADER ${SHADERS})

    # Path relative to src/shaders
    file(RELATIVE_PATH REL_PATH
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "${SHADER}"
    )

    # Final destination path
    set(DEST_FILE
            "${CMAKE_BINARY_DIR}/shaders/${REL_PATH}"
    )

    # Destination directory
    get_filename_component(DEST_DIR "${DEST_FILE}" DIRECTORY)

    add_custom_command(
        OUTPUT "${DEST_FILE}"

        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SHADER}"
                "${DEST_FILE}"

        DEPENDS "${SHADER}"

        COMMENT "Copying shader ${REL_PATH}"
    )

    list(APPEND COPIED_SHADERS "${DEST_FILE}")

endforeach()

# Target responsible for shader copying
add_custom_target(copy_shaders ALL DEPENDS ${COPIED_SHADERS})

# Define executable based on source files
add_executable(${PROJECT_NAME} ${SRC_FILES} ${HDR_FILES} ${SHADERS})

# Link external binaries
target_link_libraries(${PROJECT_NAME} glad glfw itugl ${APPLE_LIBRARIES})

# If using MSVC, choose runtime behavior
if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded"
    )
endif()
