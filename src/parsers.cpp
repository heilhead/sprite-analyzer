#include <nlohmann/json.hpp>
#include <glob/glob.h>
#include <tasks.h>
#include <mutex>
#include "parsers.h"
#include "geom.h"
#include "debug.h"
#include "png.h"
#include "util.h"

using json = nlohmann::json;

template<typename T>
json to_json(const geom::Bounds<T>& bounds) {
    return json({
        { "x", bounds.min.x },
        { "y", bounds.min.y },
        { "width", bounds.getWidth() },
        { "height", bounds.getHeight() }
    });
}

int parseSingle(const cxxopts::ParseResult& opts) {
    if (!opts.count("input")) {
        util::bail("No input file specified");
    }

    auto inFile = opts["input"].as<std::string>();

    ImageData image;

    if (!png::read(inFile.c_str(), image)) {
        util::bail("Failed to read PNG file");
    }

    auto quality = opts["optimize"].as<uint32_t>();
    if (quality > 9) {
        util::bail("Invalid quality level");
    }

    auto maxShapes = opts["max-shapes"].as<uint8_t>();
    if (maxShapes == 0) {
        util::bail("Invalid max shape count");
    }

    auto bDebug = opts.count("debug") > 0;
    auto bExtra = opts["analyze"].as<bool>();

    json result = {{ "shapes", json::array() }};

    auto numFound = image.findShapes(maxShapes);
    if (numFound > 0) {
        geom::Bounds<int> rectBounds = image.shapes[0].bounds;
        geom::Bounds<float> hullBounds;

        for (auto& object : image.shapes) {
            json shape = to_json(object.bounds);
            std::vector<glm::vec2> vertices;

            if (geom::findEnclosingPolygon(image, object, quality, vertices)) {
                shape["hull"] = json::array();

                for (auto& vertex : vertices) {
                    shape["hull"].push_back({
                        { "x", vertex.x },
                        { "y", vertex.y }
                    });

                    hullBounds.expand(vertex);
                }

                if (bDebug) {
                    debug::drawPolygon(image, vertices);
                }

                if (bExtra) {
                    shape["area"] = geom::getPolyArea(vertices);
                }
            } else {
                shape["hull"] = nullptr;
            }

            result["shapes"].push_back(shape);

            rectBounds.expand(object.bounds.min);
            rectBounds.expand(object.bounds.max);
        }

        if (bExtra) {
            result["rectBounds"] = to_json(rectBounds);

            if (hullBounds.bValid) {
                result["hullBounds"] = to_json(hullBounds);
            } else {
                result["hullBounds"] = nullptr;
            }
        }
    } else if (bExtra) {
        result["rectBounds"] = nullptr;
        result["hullBounds"] = nullptr;
    }

    if (bDebug) {
        auto outFile = opts["debug"].as<std::string>();
        if (!png::write(outFile.c_str(), image)) {
            util::bail("Failed to write debug PNG file");
        }
    }

    auto bPretty = opts["pretty"].as<bool>();

    util::print(result.dump(bPretty ? 2 : -1));

    return 0;
}

int parseMultiple(const cxxopts::ParseResult& opts) {
    struct TaskContext {
        std::mutex writeMutex;
        std::string fileName;
        ImageData image;
        geom::Bounds<int> rectBounds;
        geom::Bounds<float> hullBounds;
        json result;
        std::vector<std::vector<glm::vec2>> debugShapes;
    };

    auto workers = opts["threads"].as<uint32_t>();
    if (workers < 1) {
        util::bail("Invalid thread count");
    }

    auto quality = opts["optimize"].as<uint32_t>();
    if (quality > 9) {
        util::bail("Invalid quality level");
    }

    auto maxShapes = opts["max-shapes"].as<uint8_t>();
    if (maxShapes == 0) {
        util::bail("Invalid max shape count");
    }

    auto bDebug = opts.count("debug") > 0;
    auto bExtra = opts["analyze"].as<bool>();
    auto pattern = opts["files"].as<std::string>();

    tasks::init(workers);

    json output = {{ "files", json::array() }};

    auto root = tasks::add([&](auto& task) {
        for (auto& inFile : glob::glob(pattern)) {
            auto ctx = std::make_shared<TaskContext>();
            ctx->fileName = std::move(inFile.string());

            tasks::chain(task)
                ->add([&, ctx](auto& task) {
                    if (!png::read(ctx->fileName.c_str(), ctx->image)) {
                        ctx->result["error"] = "Failed to read PNG file";
                        return;
                    }

                    auto numFound = ctx->image.findShapes(maxShapes);
                    if (numFound > 0) {
                        ctx->result["shapes"] = json::array();

                        tasks::chain(task)
                            ->add([&, ctx](auto& task) { // image read only
                                ctx->rectBounds = ctx->image.shapes[0].bounds;

                                for (auto i = 0u; i < ctx->image.shapes.size(); i++) {
                                    tasks::add(task, [&, ctx, i](auto&) {
                                        const auto& object = ctx->image.shapes[i];
                                        json shape = to_json(object.bounds);
                                        std::vector<glm::vec2> vertices;
                                        geom::Bounds<float> tmpHullBounds;
                                        auto bFound = geom::findEnclosingPolygon(ctx->image, object, quality, vertices);

                                        if (bFound) {
                                            shape["hull"] = json::array();

                                            for (auto& vertex : vertices) {
                                                shape["hull"].push_back({
                                                    { "x", vertex.x },
                                                    { "y", vertex.y }
                                                });

                                                tmpHullBounds.expand(vertex);
                                            }

                                            if (bExtra) {
                                                shape["area"] = geom::getPolyArea(vertices);
                                            }
                                        } else {
                                            shape["hull"] = nullptr;
                                        }

                                        {
                                            // write results
                                            std::lock_guard lg(ctx->writeMutex);

                                            ctx->result["shapes"].push_back(shape);

                                            if (tmpHullBounds.bValid) {
                                                ctx->hullBounds.expand(tmpHullBounds);
                                            }

                                            ctx->rectBounds.expand(object.bounds.min);
                                            ctx->rectBounds.expand(object.bounds.max);

                                            if (bDebug) {
                                                ctx->debugShapes.emplace_back(std::move(vertices));
                                            }
                                        }
                                    });
                                }
                            })
                            ->add([&, ctx](auto&) { // image write
                                if (bExtra) {
                                    ctx->result["rectBounds"] = to_json(ctx->rectBounds);

                                    if (ctx->hullBounds.bValid) {
                                        ctx->result["hullBounds"] = to_json(ctx->hullBounds);
                                    } else {
                                        ctx->result["hullBounds"] = nullptr;
                                    }
                                }

                                if (bDebug) {
                                    for (auto& vertices : ctx->debugShapes) {
                                        debug::drawPolygon(ctx->image, vertices);
                                    }
                                }
                            })
                            ->submit();
                    } else if (bExtra) {
                        ctx->result["error"] = "No shapes found";
                        ctx->result["rectBounds"] = nullptr;
                        ctx->result["hullBounds"] = nullptr;
                    }
                })
                ->add([&, ctx](auto&) {
                    ctx->result["path"] = ctx->fileName;
                    output["files"].push_back(ctx->result);

                    if (bDebug) {
                        auto sOutFile = ctx->fileName + opts["debug"].as<std::string>();
                        if (!png::write(sOutFile.c_str(), ctx->image)) {
                            // @TODO Do what exactly?
                        }
                    }
                })
                ->submit();
        }
    });

    tasks::wait(root);
    tasks::shutdown();

    auto bPretty = opts["pretty"].as<bool>();

    util::print(output.dump(bPretty ? 2 : -1));

    return 0;
}