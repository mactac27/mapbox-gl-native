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

    const Tileset* getTileset() const { return tileset.get(); }
    std::unique_ptr<const Tileset> tileset;

private:
    virtual Range<uint8_t> getZoomRange() final;
    bool updateData(const std::string&) final;
    std::unique_ptr<TileData> createTile(const OverscaledTileID&,
                                         const UpdateParameters&,
                                         const TileLoadingCallback&) final;
};

} // namespace style
} // namespace mbgl
