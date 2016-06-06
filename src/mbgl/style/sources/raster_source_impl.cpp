#include <mbgl/style/sources/raster_source_impl.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/style/parser.hpp>
#include <mbgl/tile/raster_tile_data.hpp>

namespace mbgl {
namespace style {

RasterSource::Impl::Impl(Source& source_,
                         const std::string& id_,
                         const std::string& url_,
                         std::unique_ptr<Tileset> tileset_,
                         uint16_t tileSize_)
    : Source::Impl(source_, SourceType::Raster, id_, url_, tileSize_),
      tileset(std::move(tileset_)) {
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

bool RasterSource::Impl::updateData(const std::string& data) {
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
