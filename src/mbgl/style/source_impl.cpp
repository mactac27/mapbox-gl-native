#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/map/transform.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/renderer/painter.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/style/query_parameters.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/math/minmax.hpp>
#include <mbgl/math/clamp.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/token.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/tile_cover.hpp>

#include <mbgl/style/parser.hpp>
#include <mbgl/gl/debugging.hpp>

#include <mbgl/algorithm/update_renderables.hpp>

#include <mapbox/geometry/envelope.hpp>

#include <algorithm>

namespace mbgl {
namespace style {

static SourceObserver nullObserver;

Source::Impl::Impl(Source& source_,
                   SourceType type_,
                   const std::string& id_,
                   const std::string& url_,
                   uint16_t tileSize_,
                   std::unique_ptr<Tileset>&& tileset_)
    : type(type_),
      id(id_),
      url(url_),
      tileSize(tileSize_),
      tileset(std::move(tileset_)),
      source(source_) {
}

Source::Impl::~Impl() = default;

bool Source::Impl::isLoaded() const {
    if (!loaded) return false;

    for (const auto& pair : tileDataMap) {
        if (!pair.second->isComplete()) {
            return false;
        }
    }

    return true;
}

bool Source::Impl::isLoading() const {
    return !loaded && req.operator bool();
}

void Source::Impl::load(FileSource& fileSource) {
    if (url.empty()) {
        // In case there is no URL set, we assume that we already have all of the data because the
        // TileJSON was specified inline in the stylesheet.
        loaded = true;
        return;
    }

    if (req) {
        // We don't have a Tileset object yet, but there's already a request underway to load
        // the data.
        return;
    }

    // URL may either be a TileJSON file, or a GeoJSON file.
    req = fileSource.request(Resource::source(url), [this](Response res) {
        if (res.error) {
            observer->onSourceError(source, std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            observer->onSourceError(source, std::make_exception_ptr(std::runtime_error("unexpectedly empty source")));
        } else {
            if (updateData(*res.data)) {
                // Tile information changed because we got new GeoJSON data, or a new tile URL.
                tileDataMap.clear();
                tiles.clear();
                cache.clear();
            }

            loaded = true;
            observer->onSourceLoaded(source);
        }
    });
}

bool Source::Impl::updateData(const std::string& data) {
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

void Source::Impl::updateMatrices(const mat4 &projMatrix, const TransformState &transform) {
    for (auto& pair : tiles) {
        auto& tile = pair.second;
        transform.matrixFor(tile.matrix, tile.id);
        matrix::multiply(tile.matrix, projMatrix, tile.matrix);
    }
}

void Source::Impl::finishRender(Painter &painter) {
    for (auto& pair : tiles) {
        auto& tile = pair.second;
        painter.renderTileDebug(tile);
    }
}

const std::map<UnwrappedTileID, Tile>& Source::Impl::getTiles() const {
    return tiles;
}

TileData* Source::Impl::getTileData(const OverscaledTileID& overscaledTileID) const {
    auto it = tileDataMap.find(overscaledTileID);
    if (it != tileDataMap.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

bool Source::Impl::update(const UpdateParameters& parameters) {
    bool allTilesUpdated = true;

    if (!loaded || parameters.animationTime <= updated) {
        return allTilesUpdated;
    }

    // Determine the overzooming/underzooming amounts and required tiles.
    int32_t overscaledZoom = util::coveringZoomLevel(parameters.transformState.getZoom(), type, tileSize);
    int32_t dataTileZoom = overscaledZoom;

    std::vector<UnwrappedTileID> idealTiles;
    if (overscaledZoom >= tileset->minZoom) {
        int32_t idealZoom = std::min<int32_t>(tileset->maxZoom, overscaledZoom);

        // Make sure we're not reparsing overzoomed raster tiles.
        if (type == SourceType::Raster) {
            dataTileZoom = idealZoom;
        }

        idealTiles = util::tileCover(parameters.transformState, idealZoom);
    }

    // Stores a list of all the data tiles that we're definitely going to retain. There are two
    // kinds of tiles we need: the ideal tiles determined by the tile cover. They may not yet be in
    // use because they're still loading. In addition to that, we also need to retain all tiles that
    // we're actively using, e.g. as a replacement for tile that aren't loaded yet.
    std::set<OverscaledTileID> retain;

    auto retainTileDataFn = [&retain](const TileData& tileData) -> void {
        retain.emplace(tileData.id);
    };
    auto getTileDataFn = [this](const OverscaledTileID& dataTileID) -> TileData* {
        return getTileData(dataTileID);
    };
    auto createTileDataFn = [this, &parameters](const OverscaledTileID& dataTileID) -> TileData* {
        std::unique_ptr<TileData> data = cache.get(dataTileID);
        if (!data) {
            data = createTile(dataTileID, parameters, [=] (std::exception_ptr err) {
                tileLoadingCallback(dataTileID, err, true);
            });
        }
        if (!data) {
            return nullptr;
        }
        return tileDataMap.emplace(dataTileID, std::move(data)).first->second.get();
    };
    auto renderTileFn = [this](const UnwrappedTileID& renderTileID, TileData& tileData) {
        tiles.emplace(renderTileID, Tile{ renderTileID, tileData });
    };

    tiles.clear();
    algorithm::updateRenderables(getTileDataFn, createTileDataFn, retainTileDataFn, renderTileFn,
                                 idealTiles, *tileset, dataTileZoom);

    if (type != SourceType::Raster && type != SourceType::Annotations && cache.getSize() == 0) {
        size_t conservativeCacheSize =
            ((float)parameters.transformState.getWidth() / util::tileSize) *
            ((float)parameters.transformState.getHeight() / util::tileSize) *
            (parameters.transformState.getMaxZoom() - parameters.transformState.getMinZoom() + 1) *
            0.5;
        cache.setSize(conservativeCacheSize);
    }

    // Remove stale data tiles from the active set of tiles.
    // This goes through the (sorted!) tileDataMap and retain set in lockstep and removes items from
    // tileDataMap that don't have the corresponding key in the retain set.
    auto dataIt = tileDataMap.begin();
    auto retainIt = retain.begin();
    while (dataIt != tileDataMap.end()) {
        if (retainIt == retain.end() || dataIt->first < *retainIt) {
            cache.add(dataIt->first, std::move(dataIt->second));
            tileDataMap.erase(dataIt++);
        } else {
            if (!(*retainIt < dataIt->first)) {
                ++dataIt;
            }
            ++retainIt;
        }
    }

    for (auto& pair : tileDataMap) {
        const auto& dataTileID = pair.first;
        auto tileData = pair.second.get();
        if (parameters.shouldReparsePartialTiles && tileData->isIncomplete()) {
            auto callback = std::bind(&Source::Impl::tileLoadingCallback, this, dataTileID,
                                      std::placeholders::_1, false);

            if (!tileData->parsePending(callback)) {
                allTilesUpdated = false;
            }
        } else {
            tileData->redoPlacement({ parameters.transformState.getAngle(),
                                      parameters.transformState.getPitch(),
                                      parameters.debugOptions & MapDebugOptions::Collision },
                                    [this]() { observer->onPlacementRedone(); });
        }
    }

    updated = parameters.animationTime;

    return allTilesUpdated;
}

static Point<int16_t> coordinateToTilePoint(const UnwrappedTileID& tileID, const Point<double>& p) {
    auto zoomedCoord = TileCoordinate { p, 0 }.zoomTo(tileID.canonical.z);
    return {
        int16_t(util::clamp<int64_t>((zoomedCoord.p.x - tileID.canonical.x - tileID.wrap * std::pow(2, tileID.canonical.z)) * util::EXTENT,
                    std::numeric_limits<int16_t>::min(),
                    std::numeric_limits<int16_t>::max())),
        int16_t(util::clamp<int64_t>((zoomedCoord.p.y - tileID.canonical.y) * util::EXTENT,
                    std::numeric_limits<int16_t>::min(),
                    std::numeric_limits<int16_t>::max()))
    };
}

std::unordered_map<std::string, std::vector<Feature>> Source::Impl::queryRenderedFeatures(const QueryParameters& parameters) const {
    LineString<double> queryGeometry;

    for (const auto& p : parameters.geometry) {
        queryGeometry.push_back(TileCoordinate::fromScreenCoordinate(
            parameters.transformState, 0, { p.x, parameters.transformState.getHeight() - p.y }).p);
    }

    mapbox::geometry::box<double> box = mapbox::geometry::envelope(queryGeometry);

    std::unordered_map<std::string, std::vector<Feature>> result;

    for (const auto& tilePtr : tiles) {
        const Tile& tile = tilePtr.second;

        Point<int16_t> tileSpaceBoundsMin = coordinateToTilePoint(tile.id, box.min);
        Point<int16_t> tileSpaceBoundsMax = coordinateToTilePoint(tile.id, box.max);

        if (tileSpaceBoundsMin.x >= util::EXTENT || tileSpaceBoundsMin.y >= util::EXTENT ||
            tileSpaceBoundsMax.x < 0 || tileSpaceBoundsMax.y < 0) continue;

        GeometryCoordinates tileSpaceQueryGeometry;

        for (const auto& c : queryGeometry) {
            tileSpaceQueryGeometry.push_back(coordinateToTilePoint(tile.id, c));
        }

        tile.data.queryRenderedFeatures(result,
                                        tileSpaceQueryGeometry,
                                        parameters.transformState,
                                        parameters.layerIDs);
    }

    return result;
}

void Source::Impl::setCacheSize(size_t size) {
    cache.setSize(size);
}

void Source::Impl::onLowMemory() {
    cache.clear();
}

void Source::Impl::setObserver(SourceObserver* observer_) {
    observer = observer_;
}

void Source::Impl::tileLoadingCallback(const OverscaledTileID& tileID,
                                 std::exception_ptr error,
                                 bool isNewTile) {
    auto it = tileDataMap.find(tileID);
    if (it == tileDataMap.end()) {
        return;
    }

    auto& tileData = it->second;
    if (!tileData) {
        return;
    }

    if (error) {
        observer->onTileError(source, tileID, error);
        return;
    }

    tileData->redoPlacement([this]() {
        observer->onPlacementRedone();
    });
    observer->onTileLoaded(source, tileID, isNewTile);
}

void Source::Impl::dumpDebugLogs() const {
    Log::Info(Event::General, "Source::id: %s", id.c_str());
    Log::Info(Event::General, "Source::loaded: %d", loaded);

    for (const auto& pair : tileDataMap) {
        auto& tileData = pair.second;
        tileData->dumpDebugLogs();
    }
}

} // namespace style
} // namespace mbgl
