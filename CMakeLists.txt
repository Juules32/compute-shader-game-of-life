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
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE HDR_FILES "src/*.h")
file(GLOB_RECURSE SHADERS "src/shaders/*.*")

source_group("Shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HDR_FILES} ${SHADERS})

target_link_libraries(${PROJECT_NAME} glad glfw itugl ${APPLE_LIBRARIES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
