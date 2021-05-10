#pragma once

#include <vector>
#include <chrono>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace debug {
    struct Timer {
        std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
        std::string name;

        explicit Timer(const char* name)
            :name(name) {
        }

        ~Timer() {
            using namespace std::literals;

            auto end = std::chrono::steady_clock::now();
            auto elapsed = end - start;

            std::cout << name.c_str() << ": " << (elapsed / 1ms) << "ms" << std::endl;
        }
    };

    void drawPolygon(class ImageData& image, std::vector<glm::vec2>& vertices,
        glm::vec4 color = glm::vec4(0.5f, 0.0f, 0.0f, 1.f));
    void test();
}