#include <mbgl/style/sources/vector_source_impl.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/style/parser.hpp>
#include <mbgl/tile/vector_tile_data.hpp>
#include <mbgl/tile/vector_tile.hpp>

namespace mbgl {
namespace style {

VectorSource::Impl::Impl(Source& source_,
                         const std::string& id_,
                         const std::string& url_,
                         std::unique_ptr<Tileset> tileset_)
    : Source::Impl(source_, SourceType::Vector, id_, url_, util::tileSize),
      tileset(std::move(tileset_)) {
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

bool VectorSource::Impl::updateData(const std::string& data) {
    // Base implementation for raster and vector sources.
    bool reloadTiles = false;

    std::unique_ptr<Tileset> newTileset;

    // Create a new copy of the Tileset object that holds the base values we've parsed
    // from the stylesheet. Then merge in the values parsed from the TileJSON we retrieved
    // via the URL.
    try {
        newTileset = style::parseTileJSON(data, url, type, tileSize);
    } catch (...) {
        observer->onSourceError(source, std::current_exception());
        return false;
    }

    // Check whether previous information specifies different tile
    if (tileset && tileset->tiles != newTileset->tiles) {
        reloadTiles = true;

        // Tile size changed: We need to recalculate the tiles we need to load because we
        // might have to load tiles for a different zoom level
        // This is done automatically when we trigger the onSourceLoaded observer below.

        // Min/Max zoom changed: We need to recalculate what tiles to load, if we have tiles
        // loaded that are outside the new zoom range
        // This is done automatically when we trigger the onSourceLoaded observer below.

        // Attribution changed: We need to notify the embedding application that this
        // changed. See https://github.com/mapbox/mapbox-gl-native/issues/2723
        // This is not yet implemented.

        // Center/bounds changed: We're not using these values currently
    }

    tileset = std::move(newTileset);

    return reloadTiles;
}

} // namespace style
} // namespace mbgl
