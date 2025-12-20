#pragma once
#include "HitTable.hpp"
#include "tgaimage.hpp"
#include "Interval.hpp"
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "Sphere.hpp"

class Camera
{
    float _aspect_ratio{1.f};
    int _image_width{500};
    int _image_height{500};
    int _samples_per_pixel{10};
    int _max_depth{10};
    float _fov{90.f};
    glm::vec3 _lookfrom{0.,0.,0.};
    glm::vec3 _lookat{0.,0.,-1.};
    glm::vec3 _vup{0.,1.,0.};

    float defocus_angle = 0.f;  // Variation angle of rays through each pixel
    float focus_dist = 10.f;    // Distance from camera lookfrom point to plane of perfect focus

    float _pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    glm::vec3 _center;               // Camera center
    glm::vec3 _pixel00_loc;          // Location of pixel 0, 0
    glm::vec3 _pixel_delta_u;        // Offset to pixel to the right
    glm::vec3 _pixel_delta_v;        // Offset to pixel below
    glm::vec3 _u, _v, _w;              // Camera frame basis vectors
    glm::vec3 _defocus_disk_u;       // Defocus disk horizontal radius
    glm::vec3 _defocus_disk_v;       // Defocus disk vertical radius

    TGAColor interpolate_color(float value, const TGAColor& c1, const TGAColor& c2)
    {
        float _v = 1.f - value;
        return TGAColor
        {
            static_cast<std::uint8_t>(c1.bgra[0] * value + c2.bgra[0] * _v),
            static_cast<std::uint8_t>(c1.bgra[1] * value + c2.bgra[1] * _v),
            static_cast<std::uint8_t>(c1.bgra[2] * value + c2.bgra[2] * _v),
            255
        };
    }

    TGAColor ray_color(const Ray& light, const HitTable& world)
    {
        HitRecord record;
        if (world.hit(light, Interval{0, Interval::f_max}, record))
        {
            glm::vec3 normal = glm::normalize(record._normal);
            return TGAColor
            {
                static_cast<std::uint8_t>(122.5f * (normal.b + 1.f)),
                static_cast<std::uint8_t>(122.5f * (normal.g + 1.f)), 
                static_cast<std::uint8_t>(122.5f * (normal.r + 1.f)), 
                255
            };
        }
        glm::vec3 dir = glm::normalize(light.direction());
        return interpolate_color((1. + dir.y) * .5f, TGAColor{255,255,255,255}, TGAColor{255,179,128,255});
    }
public:
    void init()
    {

        _pixel_samples_scale = 1.f / _samples_per_pixel;

        _center = _lookfrom;

        // Determine viewport dimensions.
        auto theta = glm::radians(_fov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * _aspect_ratio;

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        _w = glm::normalize(_lookfrom - _lookat);
        _u = glm::normalize(cross(_vup, _w));
        _v = cross(_w, _u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        glm::vec3 viewport_u = viewport_width * _u;    // Vector across viewport horizontal edge
        glm::vec3 viewport_v = viewport_height * -_v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        _pixel_delta_u = viewport_u * (1.f / _image_width);
        _pixel_delta_v = viewport_v * (1.f / _image_height);

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = _center - (focus_dist * _w) - viewport_u * .5f - viewport_v * .5f;
        _pixel00_loc = viewport_upper_left + 0.5f * _pixel_delta_u + 0.5f * _pixel_delta_v;

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(glm::degrees(defocus_angle / 2));
        _defocus_disk_u = _u * defocus_radius;
        _defocus_disk_v = _v * defocus_radius;
    }

    void render(TGAImage& img, const HitTable& world)
    {
        for (int h = 0; h < _image_height; h++)
        {
            int y = _image_height - 1 - h;
            glm::vec3 height_vector = static_cast<float>(y) * _pixel_delta_v;
            for (int w = 0; w < _image_width; w++)
            {
                int x = w;
                glm::vec3 width_vector = static_cast<float>(x) * _pixel_delta_u;
                glm::vec3 pixel_center = width_vector + height_vector + _pixel00_loc;
                Ray light{_center, pixel_center - _center};
                img.set(x, y, ray_color(light, world));
            }
        }
    }

    inline int get_image_width() { return _image_width; }
    inline int get_image_height() { return _image_height; }

};