#pragma once

#include <mbgl/style/source.hpp>

namespace mbgl {

class Tileset;

namespace style {

class VectorSource : public Source {
public:
    VectorSource(const std::string& id,
                 const std::string& url,
                 std::unique_ptr<Tileset>);

    // Private implementation

    class Impl;
};

} // namespace style
} // namespace mbgl
