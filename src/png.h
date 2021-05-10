#pragma once

#include <lodepng/lodepng.h>
#include "ImageData.h"

namespace png {
    bool read(const char* filePath, ImageData& image);
    bool write(const char* filePath, ImageData& image);
}