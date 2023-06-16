cmake_minimum_required(VERSION 3.25)
project(LaplaceSmoothingVisualized)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/libs/OpenMesh/src/)
include_directories(OpenGLGame PUBLIC ${PROJECT_SOURCE_DIR}/src/libs/glfw3/gl)
include_directories(OpenGLGame PUBLIC ${PROJECT_SOURCE_DIR}/src/libs/)
include_directories(OpenGLGame PUBLIC ${PROJECT_SOURCE_DIR}src/libs/KHR)
include_directories(OpenGLGame PUBLIC ${PROJECT_SOURCE_DIR}src/libs/glm)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
set(OpenGlLinkers -lglut -lGLU -lGL -lGLEW -lm)

add_executable(LaplaceSmoothingVisualized
                src/main.cpp
                src/mesh.h
                src/libs/glad.c
                src/libs/shader.cpp
                src/util.h
                src/camera.h)

add_subdirectory(src/libs/OpenMesh)
target_link_libraries (LaplaceSmoothingVisualized
        OpenMeshCore
        OpenMeshTools
        )

target_link_libraries(LaplaceSmoothingVisualized ${OpenGlLinkers} ${CMAKE_DL_LIBS}  OpenGL::GL glfw -lX11 -lpng -lpthread -lstdc++fs -std=c++17 ) # linking opengl
