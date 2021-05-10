#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <glm/vec2.hpp>
#include "geom.h"

struct ImageShape {
    uint8_t id;
    geom::Bounds<int> bounds;

    ImageShape()
        :id(0), bounds() {
    }

    ImageShape(uint8_t id, int x, int y)
        :id(id), bounds(x, y) {
    }
};

class ImageData {
public:
    std::vector<uint8_t> rawData;
    std::vector<ImageShape> shapes;
    int width = 0;
    int height = 0;

private:
    std::vector<uint8_t> pixelShapeMap;

public:
    uint32_t findShapes(uint8_t maxShapes);
    int getIndex(int x, int y) const;
    uint8_t getShapeID(int index) const;
    uint8_t getAlpha(int index) const;

    void setPixelData(int index, uint32_t value);
    uint32_t getPixelData(int index);

private:
    void reset();
};
