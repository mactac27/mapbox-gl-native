#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/style/source_impl.hpp>

namespace mbgl {
namespace style {

class AnnotationSource : public Source {
public:
    AnnotationSource();

    // Private implementation

    class Impl;
    Impl* const impl;
};

class AnnotationSource::Impl : public Source::Impl {
public:
    Impl(Source&);

private:
    bool updateData(const std::string&) final;
    std::unique_ptr<TileData> createTile(const OverscaledTileID&,
                                         const UpdateParameters&,
                                         const TileLoadingCallback&) final;

};

} // namespace style
} // namespace mbgl
