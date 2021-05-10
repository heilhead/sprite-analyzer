#pragma once

#include <cxxopts.hpp>

int parseSingle(const cxxopts::ParseResult& opts);
int parseMultiple(const cxxopts::ParseResult& opts);
