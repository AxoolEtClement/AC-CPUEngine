cmake_minimum_required(VERSION 3.25)
project(AC-CPUEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.5.1
    GIT_SHALLOW TRUE
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external_libs/glfw
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW TRUE
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external_libs/imgui
)

FetchContent_Declare(

    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external_libs/implot
)

FetchContent_Declare(
    AC-Math
    GIT_REPOSITORY https://github.com/AxoolEtClement/AC-Math.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external_libs/AC-Math
)

FetchContent_MakeAvailable(glfw imgui implot AC-Math)


find_package(OpenGL REQUIRED)

file(GLOB IMGUI_CORE_SOURCES CONFIGURE_DEPENDS "${imgui_SOURCE_DIR}/*.cpp")

add_library(imgui STATIC
    ${IMGUI_CORE_SOURCES}
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)


add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(implot PUBLIC
    ${implot_SOURCE_DIR}
)

target_link_libraries(implot PUBLIC imgui)

# imgui n'a pas besoin de la lib math

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(AC-CPUEngine ${SOURCES} ${HEADERS})
target_include_directories(AC-CPUEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Injecter le répertoire racine du projet dans le code C++
target_compile_definitions(AC-CPUEngine PRIVATE PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# mathLibCPP est la vraie cible de la lib (elle propage mathLibSIMD et ses includes)
target_link_libraries(AC-CPUEngine PRIVATE glfw OpenGL::GL imgui implot mathLibSIMD)

