#pragma once
#include "HitTable.hpp"
#include "Utility.hpp"
#include "tgaimage.hpp"
#include "Material.hpp"
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

constexpr glm::vec3 white = glm::vec3{1.f, 1.f, 1.f};
constexpr glm::vec3 blue = glm::vec3{.5f, .7f, 1.f};

class ScatterResult;

class Camera
{
    bool _enable_hdr{true};
    bool _enable_gama{true};
    float _aspect_ratio{1.f};
    int _image_width{500};
    int _image_height{500};
    int _samples_per_pixel{500};
    int _max_depth{10};
    float _fov{45.f};
    glm::vec3 _lookfrom{0.,0.,0.};
    glm::vec3 _lookat{0.,0.,-3.f};
    glm::vec3 _vup{0.,1.,0.};

    float defocus_angle = 0.f;  // Variation angle of rays through each pixel
    float focus_dist = 10.f;    // Distance from camera lookfrom point to plane of perfect focus

    glm::vec3 _center;               // Camera center
    glm::vec3 _pixel00_loc;          // Location of pixel 0, 0
    glm::vec3 _pixel_delta_u;        // Offset to pixel to the right
    glm::vec3 _pixel_delta_v;        // Offset to pixel below
    glm::vec3 _u, _v, _w;              // Camera frame basis vectors
    glm::vec3 _defocus_disk_u;       // Defocus disk horizontal radius
    glm::vec3 _defocus_disk_v;       // Defocus disk vertical radius

    glm::vec3 interpolate_color(float value, const glm::vec3& c1, const glm::vec3& c2)
    {
        float _v = 1.f - value;
        return glm::vec3
        {
            c1.r * value + c2.r * _v,
            c1.g * value + c2.g * _v,
            c1.b * value + c2.b * _v,
        };
    }

    Ray get_ray(float x, float y)
    {
        glm::vec3 width_vector = (x + RANDOM.get_float(-0.5f, 0.5f)) * _pixel_delta_u;
        glm::vec3 height_vector = (y + RANDOM.get_float(-0.5f, 0.5f)) * _pixel_delta_v;
        glm::vec3 pixel_center = width_vector + height_vector + _pixel00_loc;
        return {_center, pixel_center - _center};
    }
    
    glm::vec3 ray_color(Ray& light, HitTable& world, int depth)
    {
        // 结束递归
        if (depth <= 0) return glm::vec3{ .0f, .0f, .0f };
        HitRecord record;
        // 未命中物体 击中背景天空盒
        if (!world.hit(light, record)) return sky_color(glm::normalize(light.direction()));
        // 计算自发光项
        glm::vec3 emitted = record._material->emitted(record._uv, record._point);
        // 计算散射光项
        auto scatter_result = record._material->scatter(light, record);
        if (!scatter_result) return emitted;
        return emitted + scatter_result._attenuation * ray_color(scatter_result._scattered_ray, world, depth - 1);
    }

    glm::vec3 sky_color(const glm::vec3& direction) { return interpolate_color((1. + direction.y) * .5f, white, blue); }
    
public:
    Camera()
    {
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

    void render(TGAImage& img, HitTableList& world)
    {
#pragma omp parallel for schedule(dynamic, 32)
        for (int h = 0; h < _image_height; h++)
        {
            int y = h;
            for (int w = 0; w < _image_width; w++)
            {
                int x = w;
                glm::vec3 color{0.f, 0.f, 0.f};
                for (int ct = 0; ct < _samples_per_pixel; ct++)
                {
                    Ray r = get_ray(static_cast<float>(x), static_cast<float>(y));
                    color += ray_color(r, world, _max_depth);
                }
                color *= (1.f / _samples_per_pixel);
                if (_enable_hdr) color = glm::vec3(color.x/(1.f+color.x), color.y/(1.f+color.y), color.z/(1.f+color.z));
                if (_enable_gama) color = glm::pow(color, glm::vec3(1.0f / 2.2f));
                img.set(x, y, color);
            }
        }
    }

    inline int get_image_width() { return _image_width; }
    inline int get_image_height() { return _image_height; }

};