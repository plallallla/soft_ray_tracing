#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>

class Texture
{
public:
    virtual ~Texture() = default;
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& point) const = 0;
};
using TexturePtr = std::shared_ptr<Texture>;

class SolidColor : public Texture
{
    glm::vec3 _albedo;
public:
    virtual ~SolidColor() = default;
    SolidColor(const glm::vec3& albedo) : _albedo{albedo} {}
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& point) const override { return _albedo; }
};

class ImgTexture : public Texture
{
    std::vector<unsigned char> _data;
    int _width{0};
    int _height{0};
    int _channel{0};
public:
    virtual ~ImgTexture() = default;
    ImgTexture(std::string_view path)
    {
        unsigned char* data = stbi_load(path.data(), &_width, &_height, &_channel, 0);
        if (!data) throw std::runtime_error(std::string{"Texture load error : "} + path.data());
        _data.assign(data, data + static_cast<size_t>(_width) * _height * _channel);
        stbi_image_free(data);
    }
    virtual glm::vec3 value(const glm::vec2& uv, const glm::vec3& point) const override
    {
        if (!_channel) return glm::vec3(0.f);
        int x = static_cast<int>(uv[0] * (_width - 1));
        int y = static_cast<int>((1.f - uv[1]) * (_height - 1));   
        size_t pixel_index = (static_cast<size_t>(y) * _width + x) * _channel;
        if (_channel >= 3) 
        {
            return glm::vec3
            {
                _data[pixel_index + 0] / 255.0f,
                _data[pixel_index + 1] / 255.0f,
                _data[pixel_index + 2] / 255.0f,
            };
        } 
        else if (_channel == 1) return glm::vec3(_data[pixel_index] / 255.0f);
        return glm::vec3(0.0f);
    }
};