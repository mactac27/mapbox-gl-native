#include <mbgl/style/sources/annotation_source.hpp>
#include <mbgl/style/update_parameters.hpp>
#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/annotation/annotation_tile.hpp>
#include <mbgl/tile/vector_tile_data.hpp>

namespace mbgl {
namespace style {

AnnotationSource::AnnotationSource()
    : Source(SourceType::Annotations, AnnotationManager::SourceID, "", util::tileSize, std::make_unique<Tileset>()) {
}

bool AnnotationSource::updateData(const std::string&) {
    return false;
}

std::unique_ptr<TileData> AnnotationSource::createTile(const OverscaledTileID& overscaledTileID,
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
