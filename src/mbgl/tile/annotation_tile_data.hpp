#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>

namespace mbgl {

class AnnotationManager;

namespace style {
class UpdateParameters;
} // namespace style

class AnnotationTileData : public GeometryTileData {
public:
    AnnotationTileData(const OverscaledTileID&,
                       std::string sourceID,
                       const style::UpdateParameters&);
    ~AnnotationTileData() override;

    void setNecessity(Necessity) final;

private:
    AnnotationManager& annotationManager;
};

} // namespace mbgl