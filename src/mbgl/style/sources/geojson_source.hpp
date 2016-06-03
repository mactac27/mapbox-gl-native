#pragma once

#include <mbgl/style/source.hpp>

namespace mapbox {
namespace geojsonvt {
class GeoJSONVT;
} // namespace geojsonvt
} // namespace mapbox

namespace mbgl {
namespace style {

class GeoJSONSource : public Source {
public:
    GeoJSONSource(const std::string& id,
                  const std::string& url,
                  std::unique_ptr<Tileset>,
                  std::unique_ptr<mapbox::geojsonvt::GeoJSONVT>);
    ~GeoJSONSource() final;


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
