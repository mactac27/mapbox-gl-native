#pragma once

#include <mbgl/style/source.hpp>

namespace mbgl {
namespace style {

class VectorSource : public Source {
public:
    VectorSource(const std::string& id,
                 const std::string& url,
                 std::unique_ptr<Tileset>);

private:
    std::unique_ptr<TileData> createTile(const OverscaledTileID&,
                                         const UpdateParameters&,
                                         const TileLoadingCallback&) final;
};

} // namespace style
} // namespace mbgl
