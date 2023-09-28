#ifndef _RIVE_SAMPLE_ATLAS_PACKER_HPP_
#define _RIVE_SAMPLE_ATLAS_PACKER_HPP_

#include "rive/span.hpp"
#include "rive/file_asset_loader.hpp"
#include "rive/math/mat2d.hpp"
#include "rive/renderer.hpp"
#include "rive/tess/sokol/sokol_tess_renderer.hpp"
#include <vector>
#include <unordered_map>

namespace rive
{
class ImageAsset;
class SokolRenderImageResource;
/// A single atlas image generated by the packer.
class SampleAtlas
{
private:
    uint32_t m_width;
    uint32_t m_height;
    std::vector<uint8_t> m_pixels;

    uint32_t m_x = 0;
    uint32_t m_y = 0;
    uint32_t m_nextY = 0;

public:
    SampleAtlas(const uint8_t* pixels, uint32_t width, uint32_t height);
    SampleAtlas(uint32_t width, uint32_t height);
    bool pack(const uint8_t* pixels, uint32_t width, uint32_t height, Mat2D& packTransform);

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    Span<const uint8_t> pixels() const { return m_pixels; }
};

struct SampleAtlasLocation
{
    std::size_t atlasIndex;
    Mat2D transform;
    // Original width & height of the image, so we can make vertex buffers for the "default renderer
    // drawImage"
    uint32_t width;
    uint32_t height;
};

/// An Atlas packer which will create multiple atlas images (SampleAtles) as
/// necessary.
class SampleAtlasPacker
{
private:
    uint32_t m_maxWidth;
    uint32_t m_maxHeight;
    std::vector<SampleAtlas*> m_atlases;
    std::unordered_map<uint32_t, SampleAtlasLocation> m_lookup;

public:
    SampleAtlasPacker(uint32_t maxWidth, uint32_t maxHeight);
    ~SampleAtlasPacker();
    // Pack the images found in the file represented by rivBytes.
    void pack(Span<const uint8_t> rivBytes);

    bool find(const ImageAsset& asset, SampleAtlasLocation* location);
    SampleAtlas* atlas(std::size_t index);
};

class SampleAtlasLoader : public FileAssetLoader
{
private:
    SampleAtlasPacker* m_packer;
    std::unordered_map<uint32_t, rive::rcp<rive::SokolRenderImageResource>> m_sharedImageResources;

public:
    SampleAtlasResolver(SampleAtlasPacker* packer);
    void loadContents(FileAsset& asset) override;
};
} // namespace rive
#endif