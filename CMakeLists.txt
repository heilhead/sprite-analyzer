cmake_minimum_required(VERSION 3.10)
project(sprite-analyzer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
        lib/lodepng/lodepng.cpp
        lib/lodepng/lodepng.h

        src/debug.cpp src/debug.h
        src/geom.cpp src/geom.h
        src/ImageData.cpp src/ImageData.h
        src/main.cpp
        src/parsers.cpp src/parsers.h
        src/png.cpp src/png.h
        src/util.cpp src/util.h)

add_subdirectory(lib/glm)
add_subdirectory(lib/random)
add_subdirectory(lib/cxxopts)
add_subdirectory(lib/json)
add_subdirectory(lib/glob)
add_subdirectory(lib/task-graph)

target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} effolkronium_random)
target_link_libraries(${PROJECT_NAME} cxxopts)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} taskgraph)
target_link_libraries(${PROJECT_NAME} Glob)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)