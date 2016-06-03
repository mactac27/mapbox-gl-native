#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/tile/raster_tile_data.hpp>

namespace mbgl {
namespace style {

RasterSource::RasterSource(const std::string& id,
                           const std::string& url,
                           std::unique_ptr<Tileset> tileset,
                           uint16_t tileSize)
    : Source(SourceType::Raster, id, url, tileSize, std::move(tileset)) {
}

std::unique_ptr<TileData> RasterSource::createTile(const OverscaledTileID& overscaledTileID,
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
