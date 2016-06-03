#include <mbgl/style/sources/vector_source.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/tile/vector_tile_data.hpp>
#include <mbgl/tile/vector_tile.hpp>

namespace mbgl {
namespace style {

VectorSource::VectorSource(const std::string& id,
                           const std::string& url,
                           std::unique_ptr<Tileset> tileset)
    : Source(SourceType::Vector, id, url, util::tileSize, std::move(tileset)) {
}

std::unique_ptr<TileData> VectorSource::createTile(const OverscaledTileID& overscaledTileID,
                                                   const UpdateParameters& parameters,
                                                   const TileLoadingCallback& callback) {
    return std::make_unique<VectorTileData>(
        overscaledTileID,
        std::make_unique<VectorTileMonitor>(overscaledTileID,
                                            parameters.pixelRatio,
                                            tileset->tiles.at(0),
                                            parameters.fileSource),
        id,
        parameters.style,
        parameters.mode,
        callback);
}

} // namespace style
} // namespace mbgl
