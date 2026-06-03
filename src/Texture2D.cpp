#include "matrixalchemy/Texture2D.hpp"

#include "matrixalchemy/Gl.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <utility>

namespace matrixalchemy
{

    Texture2D::~Texture2D()
    {
        release();
    }

    Texture2D::Texture2D(Texture2D &&other) noexcept
        : id_(std::exchange(other.id_, 0))
    {
    }

    Texture2D &Texture2D::operator=(Texture2D &&other) noexcept
    {
        if (this != &other)
        {
            release();
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    namespace
    {

        bool requiresMipmaps(unsigned int minFilter)
        {
            return minFilter == GL_NEAREST_MIPMAP_NEAREST || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_LINEAR;
        }

    } // namespace

    void Texture2D::loadFromMemory(std::span<const unsigned char> encodedImage, const TextureSampling &sampling)
    {
        release();

        int width = 0;
        int height = 0;
        int channels = 0;
        stbi_uc *pixels = stbi_load_from_memory(encodedImage.data(), static_cast<int>(encodedImage.size()), &width, &height, &channels, STBI_rgb_alpha);
        if (pixels == nullptr)
        {
            throw std::runtime_error("Failed to decode texture image.");
        }

        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(sampling.minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(sampling.magFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int>(sampling.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int>(sampling.wrapT));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        if (requiresMipmaps(sampling.minFilter))
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(pixels);
    }

    void Texture2D::release()
    {
        if (id_ != 0)
        {
            glDeleteTextures(1, &id_);
            id_ = 0;
        }
    }

    void Texture2D::bind(unsigned int textureUnit) const
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, id_);
    }

} // namespace matrixalchemy
