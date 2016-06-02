#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/sources/geojson_source_impl.hpp>

namespace mbgl {
namespace style {

GeoJSONSource::GeoJSONSource(const std::string& id)
    : Source(SourceType::GeoJSON, std::make_unique<Impl>(*this, id)) {
}

template <>
bool Source::is<GeoJSONSource>() const {
    return type == SourceType::GeoJSON;
}

} // namespace style
} // namespace mbgl
