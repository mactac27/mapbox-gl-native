#include <mbgl/style/sources/vector_source_impl.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/tile/vector_tile_data.hpp>
#include <mbgl/tile/vector_tile.hpp>

namespace mbgl {
namespace style {

VectorSource::Impl::Impl(Source& source_,
                         const std::string& id_,
                         const std::string& url_,
                         std::unique_ptr<Tileset> tileset_)
    : Source::Impl(source_, SourceType::Vector, id_, url_, util::tileSize, std::move(tileset_)) {
}

std::unique_ptr<TileData> VectorSource::Impl::createTile(const OverscaledTileID& overscaledTileID,
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
