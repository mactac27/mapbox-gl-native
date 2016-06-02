#include <mbgl/style/sources/annotation_source.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/annotation/annotation_tile.hpp>
#include <mbgl/tile/vector_tile_data.hpp>

namespace mbgl {
namespace style {

AnnotationSource::AnnotationSource()
    : Source(SourceType::Annotations, std::make_unique<Impl>(*this))
    , impl(static_cast<Impl*>(baseImpl.get())) {
}

template <>
bool Source::is<AnnotationSource>() const {
    return type == SourceType::Annotations;
}

AnnotationSource::Impl::Impl(Source& source)
    : Source::Impl(source, SourceType::Annotations, AnnotationManager::SourceID, "", util::tileSize, std::make_unique<Tileset>()) {
    enabled = true;
}

bool AnnotationSource::Impl::updateData(const std::string&) {
    return false;
}

std::unique_ptr<TileData> AnnotationSource::Impl::createTile(const OverscaledTileID& overscaledTileID,
                                                             const UpdateParameters& parameters,
                                                             const TileLoadingCallback& callback) {
    return std::make_unique<VectorTileData>(
        overscaledTileID,
        std::make_unique<AnnotationTileMonitor>(overscaledTileID, parameters.annotationManager),
        id,
        parameters.style,
        parameters.mode,
        callback);
}

} // namespace style
} // namespace mbgl
