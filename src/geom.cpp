#include <iostream>
#include <queue>
#include <glm/vec2.hpp>
#include <effolkronium/random.hpp>
#include "ImageData.h"

uint32_t gMinIterations = 100000u;
uint32_t gMaxIterations = 10000000u;

glm::ivec2 gNeighborOffsets[] {
    glm::ivec2(-1, -1),
    glm::ivec2(0, -1),
    glm::ivec2(1, -1),
    glm::ivec2(1, 0),
    glm::ivec2(1, 1),
    glm::ivec2(0, 1),
    glm::ivec2(-1, 1),
    glm::ivec2(-1, 0)
};

inline float cross2d(const glm::vec2& a, const glm::vec2& b) {
    return a.x * b.y - a.y * b.x;
}

bool getIntersection(geom::Line& a, geom::Line& b, glm::vec2& outIntersectionPoint) {
    float d = cross2d(a.direction, b.direction);

    // Lines are parallel.
    if (std::abs(d) < 1.e-8f) {
        return false;
    }

    float t = cross2d(b.direction, a.position - b.position) / d;

    // Wrong direction.
    if (t < 0.5f) {
        return false;
    }

    outIntersectionPoint = a.position + t * a.direction;

    return true;
}

uint8_t getNeighborCount(const ImageData& image, const ImageShape& shape, int x, int y) {
    auto count = 0;

    for (auto& offset : gNeighborOffsets) {
        auto nx = x + offset.x;
        auto ny = y + offset.y;

        if (shape.bounds.contains(nx, ny)) {
            auto index = image.getIndex(nx, ny);
            if (image.getAlpha(index) > 0 && image.getShapeID(index) == shape.id) {
                count++;
            }
        }
    }

    return count;
}

void findPotentialHullVertices(const ImageData& image, const ImageShape& shape, std::vector<glm::ivec2>& outVertices) {
    // Using edge detect find potential hull vertices, where the number of neighbors is less than 5.
    for (int y = shape.bounds.min.y; y <= shape.bounds.max.y; y++) {
        for (int x = shape.bounds.min.x; x <= shape.bounds.max.x; x++) {
            if (image.getAlpha(image.getIndex(x, y)) > 0 && getNeighborCount(image, shape, x, y) < 5) {
                outVertices.emplace_back(glm::ivec2(x, y));
            }
        }
    }
}

void findOptimalPolygon(const ImageData& image, const std::vector<glm::ivec2>& inVertices,
    const std::vector<int>& inIndices, uint32_t iterations, std::vector<glm::vec2>& outVertices) {

    if (inIndices.size() <= 8) {
        // Nothing to do.
        for (auto index : inIndices) {
            outVertices.emplace_back(inVertices[index]);
        }

        return;
    }

    effolkronium::random_local random;

    // Make sure that polygon generation is deterministic.
    random.seed(12345);

    std::vector<geom::Line> lines;
    lines.reserve(inIndices.size());

    for (auto i = 0; i < inIndices.size(); i++) {
        auto pos = inVertices[inIndices[i]];
        auto dir = inVertices[inIndices[(i + 1) % inIndices.size()]] - pos;
        lines.emplace_back(geom::Line(pos, dir));
    }

    uint32_t maxLineIndex = lines.size() - 1;
    auto getRandomLineIndex = [&](uint32_t startIndex) -> size_t {
        return random.get(std::min(startIndex, maxLineIndex), maxLineIndex);
    };

    auto checkBounds = [&](glm::vec2 pt) {
        return pt.x >= 0.f && pt.x <= image.width && pt.y >= 0.f && pt.y <= image.height;
    };

    outVertices.resize(8, glm::vec2(0.f, 0.f));

    auto minArea = std::numeric_limits<float>::max();

    for (auto i = 0; i < iterations; i++) {
        auto x = getRandomLineIndex(0);
        auto y = getRandomLineIndex(x + 1);

        glm::vec2 v0;
        if (getIntersection(lines[x], lines[y], v0) && checkBounds(v0)) {
            glm::vec2 v1;
            auto z = getRandomLineIndex(y + 1);
            if (getIntersection(lines[y], lines[z], v1) && checkBounds(v1)) {
                glm::vec2 v2;
                auto w = getRandomLineIndex(z + 1);
                if (getIntersection(lines[z], lines[w], v2) && checkBounds(v2)) {
                    glm::vec2 v3;
                    auto r = getRandomLineIndex(w + 1);
                    if (getIntersection(lines[w], lines[r], v3) && checkBounds(v3)) {
                        glm::vec2 v4;
                        auto s = getRandomLineIndex(r + 1);
                        if (getIntersection(lines[r], lines[s], v4) && checkBounds(v4)) {
                            glm::vec2 v5;
                            auto t = getRandomLineIndex(s + 1);
                            if (getIntersection(lines[s], lines[t], v5) && checkBounds(v5)) {
                                glm::vec2 v6;
                                auto u = getRandomLineIndex(t + 1);
                                if (getIntersection(lines[t], lines[u], v6) && checkBounds(v6)) {
                                    glm::vec2 v7;
                                    if (getIntersection(lines[u], lines[x], v7) && checkBounds(v7)) {
                                        auto u0 = v1 - v0;
                                        auto u1 = v2 - v0;
                                        auto u2 = v3 - v0;
                                        auto u3 = v4 - v0;
                                        auto u4 = v5 - v0;
                                        auto u5 = v6 - v0;
                                        auto u6 = v7 - v0;

                                        auto area =
                                            (u0.y * u1.x - u0.x * u1.y) +
                                                (u1.y * u2.x - u1.x * u2.y) +
                                                (u2.y * u3.x - u2.x * u3.y) +
                                                (u3.y * u4.x - u3.x * u4.y) +
                                                (u4.y * u5.x - u4.x * u5.y) +
                                                (u5.y * u6.x - u5.x * u6.y);

                                        if (area < minArea) {
                                            minArea = area;
                                            outVertices[0] = v0;
                                            outVertices[1] = v1;
                                            outVertices[2] = v2;
                                            outVertices[3] = v3;
                                            outVertices[4] = v4;
                                            outVertices[5] = v5;
                                            outVertices[6] = v6;
                                            outVertices[7] = v7;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (minArea == std::numeric_limits<float>::max()) {
        outVertices.clear();
    }
}

int computeDeterminant(glm::ivec2& a, glm::ivec2& b, glm::ivec2& c) {
    auto x1 = b.x - a.x;
    auto y1 = b.y - a.y;
    auto x2 = c.x - a.x;
    auto y2 = c.y - a.y;
    return x1 * y2 - y1 * x2;
}

void computeConvexHull(std::vector<glm::ivec2>& vertices, std::vector<int>& outIndices) {
    if (vertices.size() <= 4) {
        for (auto i = 0; i < vertices.size(); i++) {
            outIndices.push_back(i);
        }

        return;
    }

    auto maxInt = std::numeric_limits<int>::max();
    auto leftmostIndex = -1;
    auto leftmostPoint = glm::ivec2(maxInt, maxInt);

    for (int i = 0; i < vertices.size(); i++) {
        auto& pt = vertices[i];
        if (pt.x < leftmostPoint.x || (pt.x == leftmostPoint.x && pt.y < leftmostPoint.y)) {
            leftmostIndex = i;
            leftmostPoint = pt;
        }
    }

    auto pointOnHullIndex = leftmostIndex;
    int endPointIndex;

    do {
        outIndices.push_back(pointOnHullIndex);
        endPointIndex = 0;

        for (auto i = 1; i < vertices.size(); i++) {
            auto bIsLeft =
                computeDeterminant(vertices[endPointIndex], vertices[outIndices.back()], vertices[i]) < 0;

            if (endPointIndex == pointOnHullIndex || bIsLeft) {
                endPointIndex = i;
            }
        }

        pointOnHullIndex = endPointIndex;
    } while (endPointIndex != leftmostIndex);
}

bool geom::findEnclosingPolygon(const ImageData& image, const ImageShape& shape, uint32_t quality,
    std::vector<glm::vec2>& outVertices) {

    outVertices.clear();

    std::vector<glm::ivec2> potentialVertices;
    findPotentialHullVertices(image, shape, potentialVertices);

    if (potentialVertices.empty()) {
        return false;
    }

    std::vector<int> hullIndices;
    computeConvexHull(potentialVertices, hullIndices);

    if (hullIndices.size() < 3) {
        return false;
    }

    float alpha = (float)quality / 9.f;
    uint32_t iterations = lerp(gMinIterations, gMaxIterations, alpha * alpha);

    findOptimalPolygon(image, potentialVertices, hullIndices, iterations, outVertices);

    return !outVertices.empty();
}

bool geom::isInsidePoly(const std::vector<glm::vec2>& vertices, const glm::vec2& pt) {
    auto vertCount = vertices.size();
    auto bInside = false;

    for (int i = 0, j = vertCount - 1; i < vertCount; j = i++) {
        auto& a = vertices[i];
        auto& b = vertices[j];

        if ((a.y <= pt.y && b.y > pt.y) || (b.y <= pt.y && a.y > pt.y)) {
            auto cross = (b.x - a.x) * (pt.y - a.y) / (b.y - a.y) + a.x;
            if (cross > pt.x) {
                bInside = !bInside;
            }
        }
    }

    return bInside;
}

float geom::getPolyArea(std::vector<glm::vec2>& v) {
    auto area = 0.f;

    for (auto i = 0; i < v.size(); i++) {
        auto j = (i + 1) % v.size();
        area += v[i].x * v[j].y - v[j].x * v[i].y;
    }

    return 0.5f * std::abs(area);
}

void geom::floodFill(const int seedX, const int seedY, const std::function<bool(int, int)>& inside,
    const std::function<void(int, int)>& set) {

    std::queue<glm::ivec2> queue;

    auto push = [&](auto x, auto y) {
        queue.push(glm::ivec2(x, y));
    };

    auto pop = [&]() {
        auto value = queue.front();
        queue.pop();
        return value;
    };

    auto scan = [&](auto lx, auto rx, auto y) {
        bool added = false;
        for (auto x = lx; x <= rx; x++) {
            if (!inside(x, y)) {
                added = false;
            } else if (!added) {
                push(x, y);
                added = true;
            }
        }
    };

    push(seedX, seedY);

    while (!queue.empty()) {
        auto pt = pop();
        auto x = pt.x;
        auto y = pt.y;
        auto lx = x;

        while (inside(lx - 1, y)) {
            set(lx - 1, y);
            lx = lx - 1;
        }

        while (inside(x, y)) {
            set(x, y);
            x = x + 1;
        }

        scan(lx, x - 1, y + 1);
        scan(lx, x - 1, y - 1);
    }
}