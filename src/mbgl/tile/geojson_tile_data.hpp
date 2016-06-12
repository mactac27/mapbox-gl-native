#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>

namespace mapbox {
namespace geojsonvt {
class GeoJSONVT;
} // namespace geojsonvt
} // namespace mapbox

namespace mbgl {

namespace style {
class UpdateParameters;
} // namespace style

class GeoJSONTileData : public GeometryTileData {
public:
    GeoJSONTileData(const OverscaledTileID&,
                   std::string sourceID,
                   const style::UpdateParameters&,
                   mapbox::geojsonvt::GeoJSONVT*);
    ~GeoJSONTileData() override;

    void setNecessity(Necessity) final;
};

} // namespace mbgl
