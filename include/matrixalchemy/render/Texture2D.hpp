#pragma once

#include <span>

namespace matrixalchemy::render
{

    struct TextureSampling
    {
        unsigned int minFilter = 0;
        unsigned int magFilter = 0;
        unsigned int wrapS = 0;
        unsigned int wrapT = 0;
    };

    class Texture2D
    {
    public:
        Texture2D() = default;
        ~Texture2D();

        Texture2D(const Texture2D &) = delete;
        Texture2D &operator=(const Texture2D &) = delete;

        Texture2D(Texture2D &&other) noexcept;
        Texture2D &operator=(Texture2D &&other) noexcept;

        void loadFromMemory(std::span<const unsigned char> encodedImage, const TextureSampling &sampling);
        void release();
        void bind(unsigned int textureUnit) const;

        [[nodiscard]] bool valid() const { return id_ != 0; }

    private:
        unsigned int id_ = 0;
    };

} // namespace matrixalchemy::render
