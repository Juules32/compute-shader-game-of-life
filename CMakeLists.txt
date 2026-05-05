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

# Define executable based on source files
add_executable(${PROJECT_NAME} ${SRC_FILES} ${HDR_FILES} ${SHADERS})

# Link external binaries
target_link_libraries(${PROJECT_NAME} glad glfw itugl ${APPLE_LIBRARIES})

# Copy shaders into destination folder post-build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
