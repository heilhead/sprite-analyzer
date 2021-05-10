#pragma once

#include <vector>
#include <functional>
#include <glm/vec2.hpp>

class ImageData;
struct ImageShape;

namespace geom {
    struct Line {
        glm::vec2 position;
        glm::vec2 direction;

        Line(glm::vec2 pos, glm::vec2 dir)
            :position(pos), direction(dir) {
        }
    };

    template<typename T>
    struct Bounds {
        bool bValid;
        glm::vec<2, T> min;
        glm::vec<2, T> max;

        Bounds()
            :bValid(false) {
        }

        Bounds(T x, T y)
            :min(x, y), max(x, y), bValid(true) {
        }

        void expand(T x, T y) {
            if (!bValid) {
                min.x = max.x = x;
                min.y = max.y = y;
                bValid = true;
            } else {
                if (x < min.x) {
                    min.x = x;
                } else if (x > max.x) {
                    max.x = x;
                }

                if (y < min.y) {
                    min.y = y;
                } else if (y > max.y) {
                    max.y = y;
                }
            }
        }

        void expand(const glm::vec<2, T>& pt) {
            expand(pt.x, pt.y);
        }

        void expand(const Bounds<T>& other) {
            expand(other.min);
            expand(other.max);
        }

        bool contains(T x, T y) const {
            return x >= min.x && x <= max.x && y >= min.y && y <= max.y;
        }

        bool contains(glm::vec<2, T>& pt) const {
            return pt.x >= min.x && pt.x <= max.x && pt.y >= min.y && pt.y <= max.y;
        }

        T getWidth() const {
            return max.x - min.x;
        }

        T getHeight() const {
            return max.y - min.y;
        }
    };

    template<typename T>
    T lerp(T a, T b, float alpha) {
        return a * (1.f - alpha) + b * alpha;
    }

    bool isInsidePoly(const std::vector<glm::vec2>& vertices, const glm::vec2& pt);
    float getPolyArea(std::vector<glm::vec2>& vertices);
    bool findEnclosingPolygon(const ImageData& image, const ImageShape& shape, uint32_t quality,
        std::vector<glm::vec2>& outVertices);
    void floodFill(int seedX, int seedY, const std::function<bool(int, int)>& inside,
        const std::function<void(int, int)>& set);
}