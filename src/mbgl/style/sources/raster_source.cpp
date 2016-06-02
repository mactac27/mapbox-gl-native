#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/style/sources/raster_source_impl.hpp>

namespace mbgl {
namespace style {

RasterSource::RasterSource(const std::string& id,
                           const std::string& url,
                           std::unique_ptr<Tileset> tileset,
                           uint16_t tileSize)
    : Source(SourceType::Raster, std::make_unique<Impl>(*this, id, url, std::move(tileset), tileSize)) {
}

template <>
bool Source::is<RasterSource>() const {
    return type == SourceType::Raster;
}

} // namespace style
} // namespace mbgl
