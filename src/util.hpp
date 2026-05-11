#pragma once

#include <fstream>
#include <iosfwd>
#include <iostream>
#include <sstream>
#include <array>
#include <filesystem>
#include "ituGL/shader/Shader.h"

#ifdef _WIN32
#define NOMINMAX // Disables windows-specific macros that mess with std::min/max
#include <windows.h>
#endif

// NOT complete but should work for windows
static std::filesystem::path GetExeDir() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#else
    return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
}

static void LoadAndCompileShader(Shader& shader, const char* path) {
    // Open the file for reading
    std::ifstream file(GetExeDir() / path);
    if (!file.is_open()) {
        std::cout << "Can't find file: " << path << std::endl;
        std::cout << "Is your working directory properly set?" << std::endl;
        return;
    }

    // Dump the contents into a string
    std::stringstream stringStream;
    stringStream << file.rdbuf() << '\0';

    // Set the source code from the string
    shader.SetSource(stringStream.str().c_str());

    // Try to compile
    if (!shader.Compile()) {
        // Get errors in case of failure
        std::array<char, 2048> errors{};
        shader.GetCompilationErrors(errors);
        std::cout << "Error compiling shader: " << path << std::endl;
        std::cout << errors.data() << std::endl;
    }
}
