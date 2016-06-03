#pragma once

#include <mbgl/style/source.hpp>

namespace mbgl {
namespace style {

class AnnotationSource : public Source {
public:
    AnnotationSource();

private:
    bool updateData(const std::string&) final;
    std::unique_ptr<TileData> createTile(const OverscaledTileID&,
                                         const UpdateParameters&,
                                         const TileLoadingCallback&) final;

};

} // namespace style
} // namespace mbgl
