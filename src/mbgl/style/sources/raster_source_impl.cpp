#include <mbgl/style/sources/raster_source_impl.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/tile/raster_tile_data.hpp>

namespace mbgl {
namespace style {

RasterSource::Impl::Impl(Source& source_,
                         const std::string& id_,
                         const std::string& url_,
                         std::unique_ptr<Tileset> tileset_,
                         uint16_t tileSize_)
    : Source::Impl(source_, SourceType::Raster, id_, url_, tileSize_, std::move(tileset_)) {
}

std::unique_ptr<TileData> RasterSource::Impl::createTile(const OverscaledTileID& overscaledTileID,
                                                         const UpdateParameters& parameters,
                                                         const TileLoadingCallback& callback) {
    return std::make_unique<RasterTileData>(
        overscaledTileID,
        parameters.pixelRatio,
        tileset->tiles.at(0),
        parameters.texturePool,
        parameters.worker,
        parameters.fileSource,
        callback);
}

} // namespace style
} // namespace mbgl
