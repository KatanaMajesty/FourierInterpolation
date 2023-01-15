cmake_minimum_required(VERSION 3.20)
project(FourierInterpolation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/FourierInterpolation")
set(FI_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(IMGUI_INCLUDE_DIRS "${FI_VENDOR_DIR}/imgui")
set(IMPLOT_INCLUDE_DIRS "${FI_VENDOR_DIR}/implot")
set(GLFW_INCLUDE_DIRS "${FI_VENDOR_DIR}/glfw/include")

file(GLOB_RECURSE 
    SOURCES "${FI_SOURCE_DIR}/*.cpp")

add_subdirectory("${FI_VENDOR_DIR}/glfw")

# GLEW
find_package(GLEW REQUIRED)

# IMGUI
add_library(IMGUI STATIC)
target_sources(IMGUI
    PRIVATE
        ${IMGUI_INCLUDE_DIRS}/imgui_demo.cpp
        ${IMGUI_INCLUDE_DIRS}/imgui_draw.cpp
        ${IMGUI_INCLUDE_DIRS}/imgui_tables.cpp
        ${IMGUI_INCLUDE_DIRS}/imgui_widgets.cpp
        ${IMGUI_INCLUDE_DIRS}/imgui.cpp
        ${IMGUI_INCLUDE_DIRS}/misc/cpp/imgui_stdlib.cpp

    PRIVATE
        ${IMGUI_INCLUDE_DIRS}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_INCLUDE_DIRS}/backends/imgui_impl_glfw.cpp
    )
target_include_directories(IMGUI
    PUBLIC 
        ${IMGUI_INCLUDE_DIRS}
        ${IMGUI_INCLUDE_DIRS}/backends
        ${GLFW_INCLUDE_DIRS}
    )

# IMPLOT
add_library(IMPLOT STATIC)
target_sources(IMPLOT
    PRIVATE 
        ${IMPLOT_INCLUDE_DIRS}/implot.cpp
        ${IMPLOT_INCLUDE_DIRS}/implot_demo.cpp
        ${IMPLOT_INCLUDE_DIRS}/implot_items.cpp
    )
target_include_directories(IMPLOT
    PUBLIC 
        ${IMGUI_INCLUDE_DIRS})
target_link_libraries(IMPLOT
    PUBLIC IMGUI)

add_executable(FourierInterpolation ${SOURCES})
target_include_directories(FourierInterpolation 
    PUBLIC ${FI_SOURCE_DIR}
    PUBLIC ${GLFW_INCLUDE_DIRS}
    PUBLIC ${GLEW_INCLUDE_DIRS}
    PUBLIC ${IMGUI_INCLUDE_DIRS}
    PUBLIC ${IMPLOT_INCLUDE_DIRS}) 
target_link_libraries(FourierInterpolation 
    PUBLIC IMGUI
    PUBLIC IMPLOT
    PUBLIC glfw
    PUBLIC GL
    PUBLIC GLEW::GLEW
    )