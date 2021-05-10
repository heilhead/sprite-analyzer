#include "debug.h"
#include "ImageData.h"

int ImageData::getIndex(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return -1;
    }

    return y * width + x;
}

uint32_t ImageData::findShapes(uint8_t maxShapes) {
    assert(maxShapes > 0);

    reset();

    auto bLimited = false;
    auto shapeCounter = 0;

    auto inside = [&](auto x, auto y) {
        auto index = getIndex(x, y);
        if (index < 0) {
            return false;
        }

        return (pixelShapeMap[index] == 0) && getAlpha(index) > 0;
    };

    auto set = [&](auto x, auto y) {
        auto index = getIndex(x, y);
        pixelShapeMap[index] = shapeCounter;
        shapes.back().bounds.expand(x, y);
    };

    auto pixelLength = width * height;
    for (int i = 0; i < pixelLength; i++) {
        if (getAlpha(i) > 0 && pixelShapeMap[i] == 0) {
            auto seedX = i % width;
            auto seedY = i / width;

            if (shapeCounter == maxShapes) {
                bLimited = true;
            } else {
                ++shapeCounter;
                shapes.emplace_back(ImageShape(shapeCounter, seedX, seedY));
            }

            geom::floodFill(seedX, seedY, inside, set);
        }
    }

    if (bLimited) {
        for (int i = 0; i < pixelLength; i++) {
            auto& shapeID = pixelShapeMap[i];
            if (shapeID > 0) {
                shapeID = 1;
            }
        }

        auto& mainBounds = shapes[0].bounds;

        for (int i = 1; i < shapes.size(); i++) {
            mainBounds.expand(shapes[i].bounds);
        }

        shapes.resize(1);
    }

    return shapes.size();
}

uint8_t ImageData::getAlpha(int index) const {
    return rawData[index * 4 + 3];
}

void ImageData::setPixelData(int index, uint32_t value) {
    assert(rawData.size() > index && index >= 0);
    std::memcpy(rawData.data() + index * 4, &value, sizeof(uint32_t));
}

uint32_t ImageData::getPixelData(int index) {
    assert(rawData.size() > index && index >= 0);
    uint32_t result;
    std::memcpy(&result, rawData.data() + index * 4, sizeof(uint32_t));
    return result;
}

void ImageData::reset() {
    auto pixelCount = width * height;
    pixelShapeMap.resize(pixelCount);
    std::memset(pixelShapeMap.data(), 0, pixelCount);
    shapes.clear();
}

uint8_t ImageData::getShapeID(int index) const {
    if (index < 0) {
        return 0;
    }

    return pixelShapeMap[index];
}
