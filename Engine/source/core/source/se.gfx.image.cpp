#include "se.gfx.hpp"
#include <tinyexr/includes/tinyexr.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/image.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(disable:4996)
#include <stb/image-write.hpp>
#include <filesystem>

namespace se {
namespace image {
  auto Image::get_descriptor() noexcept -> rhi::TextureDescriptor {
    return rhi::TextureDescriptor{
      m_extend, m_mipLevels,
      m_arrayLayers, 1,
      m_dimension, m_format,
      (uint32_t)rhi::TextureUsageEnum::COPY_DST |
      (uint32_t)rhi::TextureUsageEnum::TEXTURE_BINDING,
      {m_format} };
  }

  auto Image::get_data() noexcept -> char const* {
    return &(static_cast<char const*>(m_buffer.m_data)[m_dataOffset]);
  }

  auto PNG::write_png(std::string const& path, uint32_t width,
    uint32_t height, uint32_t channel, float* data) noexcept -> void {
    stbi_write_png(path.c_str(), width, height, channel, data,
      width * channel);
  }

  auto PNG::from_png(std::string const& path) noexcept -> std::unique_ptr<Image> {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight,
      &texChannels, STBI_rgb_alpha);
    if (!pixels) {
      se::error("Image :: failed to load texture image!");
      return nullptr;
    }
    std::unique_ptr<Image> image = std::make_unique<Image>();
    image->m_buffer = se::MiniBuffer(texWidth * texHeight * sizeof(uint8_t) * 4);
    memcpy(image->m_buffer.m_data, pixels, texWidth * texHeight * sizeof(uint8_t) * 4);
    image->m_extend = uvec3{ (uint32_t)texWidth, (uint32_t)texHeight, 1 };
    image->m_format = rhi::TextureFormat::RGBA8_UNORM_SRGB;
    image->m_dimension = rhi::TextureDimension::TEX2D;
    image->m_dataSize = image->m_buffer.m_size;
    image->m_mipLevels = 1;
    image->m_arrayLayers = 1;
    image->m_subResources.push_back(Image::SubResource{ 0, 0, 0,
      uint32_t(image->m_buffer.m_size), uint32_t(texWidth), uint32_t(texHeight) });
    stbi_image_free(pixels);
    return image;
  }

  auto JPEG::write_jpeg(std::string const& path, uint32_t width,
    uint32_t height, uint32_t channel, float* data) noexcept -> void {
    stbi_write_jpg(path.c_str(), width, height, channel, data,
      width * channel);
  }

  auto JPEG::from_jpeg(std::string const& path) noexcept -> std::unique_ptr<Image> {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight,
      &texChannels, STBI_rgb_alpha);
    if (!pixels) {
      se::error("Image :: failed to load texture image!");
      return nullptr;
    }
    std::unique_ptr<Image> image = std::make_unique<Image>();
    image->m_buffer = se::MiniBuffer(texWidth * texHeight * sizeof(uint8_t) * 4);
    memcpy(image->m_buffer.m_data, pixels, texWidth * texHeight * sizeof(uint8_t) * 4);
    image->m_extend = uvec3{ (uint32_t)texWidth, (uint32_t)texHeight, 1 };
    image->m_format = rhi::TextureFormat::RGBA8_UNORM_SRGB;
    image->m_dimension = rhi::TextureDimension::TEX2D;
    image->m_dataSize = image->m_buffer.m_size;
    image->m_mipLevels = 1;
    image->m_arrayLayers = 1;
    image->m_subResources.push_back(Image::SubResource{ 0, 0, 0,
      uint32_t(image->m_buffer.m_size), uint32_t(texWidth), uint32_t(texHeight) });
    stbi_image_free(pixels);
    return image;
  }

  auto EXR::write_exr(std::string const& path, uint32_t width,
    uint32_t height, uint32_t channel, float* data) noexcept -> void {
    EXRHeader header;
    InitEXRHeader(&header);
    EXRImage image;
    InitEXRImage(&image);
    if (channel == 3) {
      image.num_channels = 3;

      std::vector<float> images[3];
      images[0].resize(width * height);
      images[1].resize(width * height);
      images[2].resize(width * height);

      // Split RGBRGBRGB... into R, G and B layer
      for (int i = 0; i < width * height; i++) {
        images[0][i] = data[4 * i + 0];
        images[1][i] = data[4 * i + 1];
        images[2][i] = data[4 * i + 2];
      }

      float* image_ptr[3];
      image_ptr[0] = &(images[2].at(0));  // B
      image_ptr[1] = &(images[1].at(0));  // G
      image_ptr[2] = &(images[0].at(0));  // R

      image.images = (unsigned char**)image_ptr;
      image.width = width;
      image.height = height;

      header.num_channels = 3;
      header.channels =
        (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
      // Must be (A)BGR order, since most of EXR viewers expect this channel order.
      strncpy(header.channels[0].name, "B", 255);
      header.channels[0].name[strlen("B")] = '\0';
      strncpy(header.channels[1].name, "G", 255);
      header.channels[1].name[strlen("G")] = '\0';
      strncpy(header.channels[2].name, "R", 255);
      header.channels[2].name[strlen("R")] = '\0';

      header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
      header.requested_pixel_types =
        (int*)malloc(sizeof(int) * header.num_channels);
      for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] =
          TINYEXR_PIXELTYPE_FLOAT;  // pixel type of input image
        header.requested_pixel_types[i] =
          TINYEXR_PIXELTYPE_HALF;  // pixel type of output image to be stored in
        // .EXR
      }

      const char* err = NULL;  // or nullptr in C++11 or later.
      int ret = SaveEXRImageToFile(&image, &header, path.c_str(), &err);
      if (ret != TINYEXR_SUCCESS) {
        se::error("Save EXR err: " + std::string(err));
        FreeEXRErrorMessage(err);  // free's buffer for an error message
        return;
      }
    }
    else if (channel == 4) {
      image.num_channels = 4;

      std::vector<float> images[4];
      images[0].resize(width * height);
      images[1].resize(width * height);
      images[2].resize(width * height);
      images[3].resize(width * height);

      // Split RGBRGBRGB... into R, G and B layer
      for (int i = 0; i < width * height; i++) {
        images[0][i] = data[4 * i + 0];
        images[1][i] = data[4 * i + 1];
        images[2][i] = data[4 * i + 2];
        images[3][i] = data[4 * i + 3];
      }

      float* image_ptr[4];
      image_ptr[0] = &(images[3].at(0));  // A
      image_ptr[1] = &(images[2].at(0));  // B
      image_ptr[2] = &(images[1].at(0));  // G
      image_ptr[3] = &(images[0].at(0));  // R

      image.images = (unsigned char**)image_ptr;
      image.width = width;
      image.height = height;

      header.num_channels = 4;
      header.channels =
        (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
      // Must be (A)BGR order, since most of EXR viewers expect this channel order.
      strncpy(header.channels[0].name, "A", 255);
      header.channels[3].name[strlen("A")] = '\0';
      strncpy(header.channels[1].name, "B", 255);
      header.channels[0].name[strlen("B")] = '\0';
      strncpy(header.channels[2].name, "G", 255);
      header.channels[1].name[strlen("G")] = '\0';
      strncpy(header.channels[3].name, "R", 255);
      header.channels[2].name[strlen("R")] = '\0';

      header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
      header.requested_pixel_types =
        (int*)malloc(sizeof(int) * header.num_channels);
      for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] =
          TINYEXR_PIXELTYPE_FLOAT;  // pixel type of input image
        header.requested_pixel_types[i] =
          TINYEXR_PIXELTYPE_FLOAT;  // pixel type of output image to be stored in
        // .EXR
      }

      const char* err = NULL;  // or nullptr in C++11 or later.
      int ret = SaveEXRImageToFile(&image, &header, path.c_str(), &err);
      if (ret != TINYEXR_SUCCESS) {
        se::error("Save EXR err: " + std::string(err));
        FreeEXRErrorMessage(err);  // free's buffer for an error message
        return;
      }
    }

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
  }

  auto EXR::from_exr(std::string const& path) noexcept
    -> std::unique_ptr<Image> {
    const char* input = path.c_str();
    float* out;  // width * height * RGBA
    int width;
    int height;
    const char* err = nullptr;

    int ret = LoadEXR(&out, &width, &height, input, &err);

    if (ret != TINYEXR_SUCCESS) {
      if (err) {
        fprintf(stderr, "ERR : %s\n", err);
        FreeEXRErrorMessage(err);  // release memory of error message.
      }
    }
    else {
      std::unique_ptr<Image> image = std::make_unique<Image>();
      image->m_buffer = se::MiniBuffer(width * height * sizeof(float) * 4);
      memcpy(image->m_buffer.m_data, out, width * height * sizeof(float) * 4);
      image->m_extend = uvec3{ (uint32_t)width, (uint32_t)height, 1 };
      image->m_format = rhi::TextureFormat::RGBA32_FLOAT;
      image->m_dimension = rhi::TextureDimension::TEX2D;
      image->m_dataSize = image->m_buffer.m_size;
      image->m_mipLevels = 1;
      image->m_arrayLayers = 1;
      image->m_subResources.push_back(Image::SubResource{ 0, 0, 0,
        uint32_t(image->m_buffer.m_size), uint32_t(width), uint32_t(height) });
      free(out);
      return image;
    }
    return nullptr;
  }

  auto Binary::from_binary(int texWidth, int texHeight, int texChannels, int bits,
    const char* pixels) noexcept -> std::unique_ptr<Image> {
    std::unique_ptr<Image> image = std::make_unique<Image>();
    image->m_buffer = se::MiniBuffer(texWidth * texHeight * sizeof(uint8_t) * 4);
    memcpy(image->m_buffer.m_data, pixels, texWidth * texHeight * sizeof(uint8_t) * 4);
    image->m_extend = uvec3{ (uint32_t)texWidth, (uint32_t)texHeight, 1 };
    image->m_format = rhi::TextureFormat::RGBA8_UNORM_SRGB;
    image->m_dimension = rhi::TextureDimension::TEX2D;
    image->m_dataSize = image->m_buffer.m_size;
    image->m_mipLevels = 1;
    image->m_arrayLayers = 1;
    image->m_subResources.push_back(Image::SubResource{ 0, 0, 0,
      uint32_t(image->m_buffer.m_size), uint32_t(texWidth), uint32_t(texHeight) });
    return image;
  }

  auto load_image(std::string const& file_path) noexcept -> std::unique_ptr<Image> {
    std::filesystem::path path = file_path;
    if (path.extension() == ".jpg" || path.extension() == ".JPG" ||
      path.extension() == ".JPEG") {
      return JPEG::from_jpeg(file_path);
    }
    else if (path.extension() == ".png" || path.extension() == ".PNG") {
      return PNG::from_png(file_path);
    }
    else if (path.extension() == ".exr") {
      return EXR::from_exr(file_path);
    }
    else {
      se::error("Image :: Image Loader failed when loading {0}, \
    as format extension {1} not supported. ", path.string(), path.extension().string());
    }
    return nullptr;
  }
}
}