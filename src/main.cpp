#include <cxxopts.hpp>
#include <thread>
#include <string>
#include "parsers.h"
#include "util.h"

cxxopts::ParseResult initOptions(int argc, char** argv) {
    using namespace cxxopts;

    Options opts("sprite-analyzer", "Analyze images and generate polygons for each shape detected");

    auto threadNum = std::to_string(std::thread::hardware_concurrency());

    opts.add_options()
            ("i,input", "Input PNG image.", value<std::string>())
            ("f,files", "Glob pattern for multiple input files.", value<std::string>())
            ("t,threads", "Thread number.", value<uint32_t>()->default_value(threadNum))
            ("o,optimize", "Optimization level. (0-9)", value<uint32_t>()->default_value("0"))
            ("a,analyze", "Add extended analysis data.", value<bool>()->default_value("false"))
            ("d,debug", "Output debug PNG. File name for single file, or suffix for multiple files.",
                value<std::string>())
            ("m,max-shapes",
                "Maximum shapes to generate polygons for. If there's more shapes detected on the image, they'll be combined into one. (1-255)",
                value<uint8_t>()->default_value("255"))
            ("p,pretty", "Prettify generated JSON.", value<bool>()->default_value("false"))
            ("h,help", "Print usage.");

    auto result = opts.parse(argc, argv);

    if (result.count("help")) {
        util::bail(opts.help().c_str(), 0);
    }

    return result;
}

int main(int argc, char** argv) {
    auto opts = initOptions(argc, argv);

    if (opts.count("files")) {
        return parseMultiple(opts);
    } else {
        return parseSingle(opts);
    }
}