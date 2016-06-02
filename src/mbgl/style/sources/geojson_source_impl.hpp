#pragma once

#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/sources/geojson_source.hpp>

namespace mapbox {
namespace geojsonvt {
class GeoJSONVT;
} // namespace geojsonvt
} // namespace mapbox

namespace mbgl {
namespace style {

class GeoJSONSource::Impl : public Source::Impl {
public:
    Impl(Source&,
         const std::string& id);
    Impl(Source&,
         const std::string& id,
         const std::string& url,
         std::unique_ptr<Tileset>,
         std::unique_ptr<mapbox::geojsonvt::GeoJSONVT>);
    ~Impl() final;

private:
    bool updateData(const std::string&) final;
    std::unique_ptr<TileData> createTile(const OverscaledTileID&,
                                         const UpdateParameters&,
                                         const TileLoadingCallback&) final;

    std::unique_ptr<mapbox::geojsonvt::GeoJSONVT> geojsonvt;
};

std::unique_ptr<GeoJSONSource> parseGeoJSONSource(const std::string& id, const JSValue&);

} // namespace style
} // namespace mbgl
