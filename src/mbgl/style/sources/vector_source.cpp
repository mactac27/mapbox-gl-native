#include <mbgl/style/sources/vector_source.hpp>
#include <mbgl/style/sources/vector_source_impl.hpp>

namespace mbgl {
namespace style {

VectorSource::VectorSource(const std::string& id,
                           const std::string& url,
                           std::unique_ptr<Tileset> tileset)
    : Source(SourceType::Vector, std::make_unique<Impl>(*this, id, url, std::move(tileset))) {
}

template <>
bool Source::is<VectorSource>() const {
    return type == SourceType::Vector;
}

} // namespace style
} // namespace mbgl
