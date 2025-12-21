#pragma once
#include "HitTable.hpp"
#include "Utility.hpp"
#include "tgaimage.hpp"
#include "Interval.hpp"
#include "Material.hpp"
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

constexpr glm::vec3 white = glm::vec3{1.f, 1.f, 1.f};
constexpr glm::vec3 blue = glm::vec3{.5f, .7f, 1.f};

class ScatterResult;

class Camera
{
    float _aspect_ratio{1.f};
    int _image_width{500};
    int _image_height{500};
    int _samples_per_pixel{10};
    int _max_depth{1};
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


const glm::vec3 Sky(const Ray & ray) 
{
	auto normDir = glm::normalize(ray.direction());
	float t = 0.5f * (normDir.y + 1.0f);

	const glm::vec3 white(1.f);
	const glm::vec3 blue(0.5, 0.7, 1);

	return white * t + blue * (1.f - t); // 线性插值
}

glm::vec3 RandInSphere() 
{
    while (true)
    {
        glm::vec3 p
        {
            RANDOM.get_float(-1.f, 1.f),
            RANDOM.get_float(-1.f, 1.f),
            RANDOM.get_float(-1.f, 1.f),
        };            
        if (glm::dot(p, p) < 1.f) return p;
    }
}

const glm::vec3 Trace(Ray& ray, const HitTableList& scene) 
{
	HitRecord rec;
	if (scene.hit(ray, rec)) 
    {
		glm::vec3 dir = glm::normalize(rec._normal + RandInSphere()); // 漫反射
		Ray newRay(rec._point, dir);
		return 0.5f * Trace(newRay, scene); // 光照衰减一半
	}
	return Sky(ray);
}

    
    glm::vec3 ray_color(const Ray& light, const HitTable& world, int depth)
    {
        if (depth <= 0) return glm::vec3{ .0f, .0f, .0f };
        HitRecord record;
        // if (world.hit(light, Interval{0, Interval::f_max}, record))
        // {
        //     glm::vec3 dir = glm::normalize(record._normal + RandInSphere());
        //     return 0.5f * ray_color({record._point, dir}, world, depth - 1);
        // }
        return sky_color(glm::normalize(light.direction()));
    }

    glm::vec3 sky_color(const glm::vec3& direction)
    {
        return interpolate_color((1. + direction.y) * .5f, white, blue);
    }
public:
    Camera()
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

    void render(TGAImage& img, const HitTableList& world)
    {
#pragma omp parallel for
        for (int h = 0; h < _image_height; h++)
        {
            int y = h;
            for (int w = 0; w < _image_width; w++)
            {
                int x = w;
                glm::vec3 color{0.f, 0.f, 0.f};
                for (int ct = 0; ct < _samples_per_pixel; ct++)
                {
                    // color += ray_color(get_ray(static_cast<float>(x), static_cast<float>(y)), world, _max_depth);
                    Ray r = get_ray(static_cast<float>(x), static_cast<float>(y));
                    color += Trace(r, world);
                }
                img.set(x, y, color * (1.f / _samples_per_pixel));
            }
        }
    }

    inline int get_image_width() { return _image_width; }
    inline int get_image_height() { return _image_height; }

};