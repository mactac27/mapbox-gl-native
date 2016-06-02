#pragma once

#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/sources/raster_source.hpp>

namespace mbgl {
namespace style {

class RasterSource::Impl : public Source::Impl {
public:
    Impl(Source&,
         const std::string& id,
         const std::string& url,
         std::unique_ptr<Tileset>,
         uint16_t tileSize);

private:
    std::unique_ptr<TileData> createTile(const OverscaledTileID&,
                                         const UpdateParameters&,
                                         const TileLoadingCallback&) final;
};

} // namespace style
} // namespace mbgl
