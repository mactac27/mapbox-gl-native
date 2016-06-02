#pragma once

#include <mbgl/style/source.hpp>

namespace mbgl {

class Tileset;

namespace style {

class RasterSource : public Source {
public:
    RasterSource(const std::string& id,
                 const std::string& url,
                 std::unique_ptr<Tileset>,
                 uint16_t tileSize);

    // Private implementation

    class Impl;
};

} // namespace style
} // namespace mbgl
