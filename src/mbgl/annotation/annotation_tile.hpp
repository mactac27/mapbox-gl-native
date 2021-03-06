#pragma once

#include <mbgl/tile/geometry_tile.hpp>

namespace mbgl {

class AnnotationTileFeature : public GeometryTileFeature {
public:
    AnnotationTileFeature(FeatureType, GeometryCollection,
                          std::unordered_map<std::string, std::string> properties = {{}});

    FeatureType getType() const override { return type; }
    optional<Value> getValue(const std::string&) const override;
    GeometryCollection getGeometries() const override { return geometries; }

    const FeatureType type;
    const std::unordered_map<std::string, std::string> properties;
    const GeometryCollection geometries;
};

class AnnotationTileLayer : public GeometryTileLayer {
public:
    AnnotationTileLayer(const std::string&);

    std::size_t featureCount() const override { return features.size(); }
    util::ptr<const GeometryTileFeature> getFeature(std::size_t i) const override { return features[i]; }
    std::string getName() const override { return name; };

    std::vector<util::ptr<const AnnotationTileFeature>> features;

private:
    std::string name;
};

class AnnotationTile : public GeometryTile {
public:
    util::ptr<GeometryTileLayer> getLayer(const std::string&) const override;

    std::unordered_map<std::string, util::ptr<AnnotationTileLayer>> layers;
};

} // namespace mbgl
