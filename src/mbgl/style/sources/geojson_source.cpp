#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/tile/vector_tile_data.hpp>
#include <mbgl/tile/geojson_tile.hpp>
#include <mbgl/platform/log.hpp>

#include <mapbox/geojsonvt.hpp>
#include <mapbox/geojsonvt/convert.hpp>

#include <rapidjson/error/en.h>

#include <sstream>

namespace mbgl {
namespace style {

std::unique_ptr<mapbox::geojsonvt::GeoJSONVT> parseGeoJSON(const JSValue& value) {
    using namespace mapbox::geojsonvt;

    Options options;
    options.buffer = util::EXTENT / util::tileSize * 128;
    options.extent = util::EXTENT;

    try {
        return std::make_unique<GeoJSONVT>(Convert::convert(value, 0), options);
    } catch (const std::exception& ex) {
        Log::Error(Event::ParseStyle, "Failed to parse GeoJSON data: %s", ex.what());
        // Create an empty GeoJSON VT object to make sure we're not infinitely waiting for
        // tiles to load.
        return std::make_unique<GeoJSONVT>(std::vector<ProjectedFeature>{}, options);
    }
}

std::unique_ptr<GeoJSONSource> parseGeoJSONSource(const std::string& id,
                                                  const JSValue& sourceVal) {
    std::unique_ptr<Tileset> tileset = std::make_unique<Tileset>();
    std::unique_ptr<mapbox::geojsonvt::GeoJSONVT> geojsonvt;
    std::string url;

    // We should probably split this up to have URLs in the url property, and actual data
    // in the data property. Until then, we're going to detect the content based on the
    // object type.
    if (sourceVal.HasMember("data")) {
        const JSValue& dataVal = sourceVal["data"];
        if (dataVal.IsString()) {
            // We need to load an external GeoJSON file
            url = { dataVal.GetString(), dataVal.GetStringLength() };
        } else if (dataVal.IsObject()) {
            // We need to parse dataVal as a GeoJSON object
            geojsonvt = parseGeoJSON(dataVal);
            tileset->maxZoom = geojsonvt->options.maxZoom;
        } else {
            Log::Error(Event::ParseStyle, "GeoJSON data must be a URL or an object");
            return nullptr;
        }
    } else {
        Log::Error(Event::ParseStyle, "GeoJSON source must have a data value");
        return nullptr;
    }

    return std::make_unique<GeoJSONSource>(id, url, std::move(tileset), std::move(geojsonvt));
}

GeoJSONSource::GeoJSONSource(const std::string& id,
                             const std::string& url,
                             std::unique_ptr<Tileset> tileset,
                             std::unique_ptr<mapbox::geojsonvt::GeoJSONVT> geojsonvt_)
    : Source(SourceType::GeoJSON, id, url, util::tileSize, std::move(tileset)),
      geojsonvt(std::move(geojsonvt_)) {
}

GeoJSONSource::~GeoJSONSource() = default;

bool GeoJSONSource::updateData(const std::string& data) {
    std::unique_ptr<Tileset> newTileset = std::make_unique<Tileset>();

    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> d;
    d.Parse<0>(data.c_str());

    if (d.HasParseError()) {
        std::stringstream message;
        message << d.GetErrorOffset() << " - " << rapidjson::GetParseError_En(d.GetParseError());
        observer->onSourceError(*this, std::make_exception_ptr(std::runtime_error(message.str())));
        return false;
    }

    geojsonvt = style::parseGeoJSON(d);
    newTileset->maxZoom = geojsonvt->options.maxZoom;
    tileset = std::move(newTileset);

    return true;
}

std::unique_ptr<TileData> GeoJSONSource::createTile(const OverscaledTileID& overscaledTileID,
                                                    const UpdateParameters& parameters,
                                                    const TileLoadingCallback& callback) {
    return std::make_unique<VectorTileData>(
        overscaledTileID,
        std::make_unique<GeoJSONTileMonitor>(geojsonvt.get(), overscaledTileID),
        id,
        parameters.style,
        parameters.mode,
        callback);
}

} // namespace style
} // namespace mbgl
