#include <glm/vec4.hpp>
#include <numeric>
#include "geom.h"
#include "png.h"
#include "debug.h"

std::vector<glm::vec2> gCrossPixels {
    { 0.f, 0.f },
    { -1.f, 0.f },
    { 1.f, 0.f },
    { 0.f, -1.f },
    { 0.f, 1.f },
};

glm::vec4 uint2vec(uint32_t value) {
    glm::vec4 color;
    color.r = (float)((value & 0xFF000000) >> 24) / 255.f;
    color.g = (float)((value & 0x00FF0000) >> 16) / 255.f;
    color.b = (float)((value & 0x0000FF00) >> 8) / 255.f;
    color.a = (float)(value & 0x000000FF) / 255.f;
    return color;
}

uint32_t vec2uint(glm::vec4 value) {
    auto r = (uint8_t)(value.r * 255.f);
    auto g = (uint8_t)(value.g * 255.f);
    auto b = (uint8_t)(value.b * 255.f);
    auto a = (uint8_t)(value.a * 255.f);
    return r << 24 | g << 16 | b << 8 | a;
}

void debug::drawPolygon(ImageData& image, std::vector<glm::vec2>& vertices, glm::vec4 color) {
    if (vertices.size() < 3) {
        return;
    }

    auto center = std::accumulate(vertices.begin(), vertices.end(), glm::vec2(0.f)) / (float)vertices.size();
    std::vector<bool> mask(image.width * image.height, false);

    int i = 0u;

    auto inside = [&](auto x, auto y) {
        auto index = image.getIndex(x, y);
        if (index < 0 || mask[index]) {
            return false;
        }

        return geom::isInsidePoly(vertices, glm::vec2(x, y));
    };

    auto set = [&](auto x, auto y) {
        auto index = image.getIndex(x, y);
        mask[index] = true;
        auto pixelColor = geom::lerp(uint2vec(image.getPixelData(index)), color, 0.5f);
        image.setPixelData(index, vec2uint(pixelColor));
        i++;
    };

    geom::floodFill(center.x, center.y, inside, set);

    auto drawPoint = [&](auto pt, auto color) {
        for (auto& cpt : gCrossPixels) {
            auto index = image.getIndex(cpt.x + pt.x, cpt.y + pt.y);
            if (index >= 0) {
                image.setPixelData(index, color);
            }
        }
    };

    for (auto& pt : vertices) {
        drawPoint(pt, 0xFFFF00FF);
    }
    drawPoint(center, 0x00FF00FF);
}

void debug::test() {
    assert(uint2vec(0xFF000000) == glm::vec4(1.f, 0.f, 0.f, 0.f));
    assert(uint2vec(0x00FF0000) == glm::vec4(0.f, 1.f, 0.f, 0.f));
    assert(uint2vec(0x0000FF00) == glm::vec4(0.f, 0.f, 1.f, 0.f));
    assert(uint2vec(0x000000FF) == glm::vec4(0.f, 0.f, 0.f, 1.f));
    assert(vec2uint(glm::vec4(1.f, 0.f, 0.f, 0.f)) == 0xFF000000);
    assert(vec2uint(glm::vec4(0.f, 1.f, 0.f, 0.f)) == 0x00FF0000);
    assert(vec2uint(glm::vec4(0.f, 0.f, 1.f, 0.f)) == 0x0000FF00);
    assert(vec2uint(glm::vec4(0.f, 0.f, 0.f, 1.f)) == 0x000000FF);
    assert(uint2vec(0xFFFFFFFF) == glm::vec4(1.f, 1.f, 1.f, 1.f));
    assert(vec2uint(glm::vec4(1.f, 1.f, 1.f, 1.f)) == 0xFFFFFFFF);

    auto poly = std::vector<glm::vec2> {
        glm::vec2(-3.f, -2.f),
        glm::vec2(-1.f, 4.f),
        glm::vec2(6.f, 1.f),
        glm::vec2(3.f, 10.f),
        glm::vec2(-4.f, 9.f),
    };

    assert(geom::getPolyArea(poly) == 60.f);
}
