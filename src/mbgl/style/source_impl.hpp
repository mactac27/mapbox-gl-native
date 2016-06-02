#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/tile/tile_data.hpp>
#include <mbgl/tile/tile_cache.hpp>

#include <mbgl/util/mat4.hpp>
#include <mbgl/util/rapidjson.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/tileset.hpp>

#include <forward_list>
#include <vector>
#include <map>

namespace mbgl {

class Painter;
class FileSource;
class AsyncRequest;
class TransformState;
class Tile;
struct ClipID;

namespace style {

class Style;
class UpdateParameters;
class QueryParameters;
class SourceObserver;

class Source::Impl : private util::noncopyable {
protected:
    Impl(Source&,
         SourceType,
         const std::string& id,
         const std::string& url,
         uint16_t tileSize,
         std::unique_ptr<Tileset>&&);

public:
    virtual ~Impl();

    bool loaded = false;
    void load(FileSource&);
    bool isLoading() const;
    bool isLoaded() const;

    const Tileset* getTileset() const { return tileset.get(); }

    // Request or parse all the tiles relevant for the "TransformState". This method
    // will return true if all the tiles were scheduled for updating of false if
    // they were not. shouldReparsePartialTiles must be set to "true" if there is
    // new data available that a tile in the "partial" state might be interested at.
    bool update(const UpdateParameters&);

    template <typename ClipIDGenerator>
    void updateClipIDs(ClipIDGenerator& generator) {
        generator.update(tiles);
    }

    void updateMatrices(const mat4 &projMatrix, const TransformState &transform);
    void finishRender(Painter &painter);

    const std::map<UnwrappedTileID, Tile>& getTiles() const;

    TileData* getTileData(const OverscaledTileID&) const;

    std::unordered_map<std::string, std::vector<Feature>>
    queryRenderedFeatures(const QueryParameters&) const;

    void setCacheSize(size_t);
    void onLowMemory();

    void setObserver(SourceObserver* observer);
    void dumpDebugLogs() const;

    const SourceType type;
    const std::string id;
    const std::string url;
    uint16_t tileSize = util::tileSize;
    bool enabled = false;

protected:
    using TileLoadingCallback = std::function<void (std::exception_ptr)>;

    std::unique_ptr<const Tileset> tileset;

    Source& source;
    SourceObserver* observer = nullptr;

private:
    virtual bool updateData(const std::string& data);
    virtual std::unique_ptr<TileData> createTile(const OverscaledTileID&,
                                                 const UpdateParameters&,
                                                 const TileLoadingCallback&) = 0;

    void tileLoadingCallback(const OverscaledTileID&, std::exception_ptr, bool isNewTile);


    // Stores the time when this source was most recently updated.
    TimePoint updated = TimePoint::min();

    std::map<UnwrappedTileID, Tile> tiles;
    std::map<OverscaledTileID, std::unique_ptr<TileData>> tileDataMap;
    TileCache cache;

    std::unique_ptr<AsyncRequest> req;
};

} // namespace style
} // namespace mbgl
