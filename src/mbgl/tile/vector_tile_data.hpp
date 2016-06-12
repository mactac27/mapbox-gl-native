#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/tile_source.hpp>

namespace mbgl {

class Tileset;
class GeometryTile;

namespace style {
class UpdateParameters;
} // namespace style

class VectorTileData : public GeometryTileData {
public:
    VectorTileData(const OverscaledTileID&,
                   std::string sourceID,
                   const style::UpdateParameters&,
                   const Tileset&);
    ~VectorTileData() override;

    void setNecessity(Necessity) final;

    static std::unique_ptr<GeometryTile> parseData(std::shared_ptr<const std::string>);

private:
    TileSource<VectorTileData> tileSource;
};

} // namespace mbgl
