#include "png.h"

bool png::read(const char* filePath, ImageData& image) {
    uint32_t width, height;
    auto error = lodepng::decode(image.rawData, width, height, filePath);
    if (error) {
        return false;
    }

    image.width = static_cast<int>(width);
    image.height = static_cast<int>(height);

    return true;
}

bool png::write(const char* filePath, ImageData& image) {
    auto error = lodepng::encode(filePath, image.rawData, image.width, image.height);
    if (error) {
        return false;
    }

    return true;
}