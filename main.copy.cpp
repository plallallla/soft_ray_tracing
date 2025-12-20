#include <cmath>
#include <csetjmp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <algorithm>
#include "Ray.hpp"
#include "tgaimage.hpp"


struct Data
{
    float aspect_ratio = 16.f / 9.f;
    int image_width = 640;
    int image_height = static_cast<int>(640 / 16.f * 9.f);  
    float camera_focal_length{1.f};
    glm::vec3 camera_center{.0f,.0f,.0f};      
    float viewport_h{2.f};
    float viewport_w{viewport_h * aspect_ratio};    
    glm::vec3 viewport_u = glm::vec3(viewport_w, 0, 0);
    glm::vec3 viewport_v = glm::vec3(0, -1.f * viewport_h, 0);
    glm::vec3 pixel_delta_u = viewport_u * (1.f / image_width);
    glm::vec3 pixel_delta_v = viewport_v * (1.f / image_height);
    glm::vec3 viewport_upper_left = 
    camera_center - glm::vec3(0,0,camera_focal_length) - viewport_u * .5f - viewport_v * .5f;
    glm::vec3 pixel_100_Loc = viewport_upper_left + .5f * (pixel_delta_u + pixel_delta_v);
};

Data some_pre_data;


struct Sphere
{
    glm::vec3 _center;
    float _radius;
};



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

Sphere s{glm::vec3{0, 0, -1.f}, .5f};

bool hit_sphere(const Sphere& s, const Ray& r, float& t)
{
    glm::vec3 ori = r.origin() - s._center;
    float a = glm::dot(r.direction(), r.direction());
    float b = 2.f * glm::dot(ori, r.direction());
    float c = glm::dot(ori, ori) - s._radius * s._radius;
    float delta = b * b - 4 * a * c;
    if (delta < 0)
    {
        return false;
    }
    t = (-1.f * b - sqrt(delta)) / (2.f * a);
    return true;
}

TGAColor RayColor(const Ray ray)
{
    float t;
    if (hit_sphere(s, ray, t))
    {
        glm::vec3 normal = glm::normalize(ray.at(t) - s._center);
        return TGAColor
        {
            static_cast<std::uint8_t>(255 * 0.5 * (normal.b + 1.f)), 
            static_cast<std::uint8_t>(255 * 0.5 * (normal.g + 1.f)), 
            static_cast<std::uint8_t>(255 * 0.5 * (normal.r + 1.f)),
            255
        };
    }
    auto n_dir = glm::normalize(ray.direction());
    return interpolate_color((1. + n_dir.y) * .5f, TGAColor{255,255,255,255}, TGAColor{255,179,128,255});

}

int main(int, char**){
    TGAImage framebuffer(some_pre_data.image_width, some_pre_data.image_height, TGAImage::RGB);

    for (int w = 0; w < some_pre_data.image_width; w++)
    {
        for (int h = 0; h < some_pre_data.image_height; h++)
        {
            auto pixel_center = some_pre_data.pixel_100_Loc + 
            1.f * h * some_pre_data.pixel_delta_v + 
            1.f * w * some_pre_data.pixel_delta_u;
            glm::vec3 ray_direction = pixel_center - some_pre_data.camera_center;
            Ray ray{some_pre_data.camera_center, ray_direction};
            framebuffer.set(w, h, RayColor(ray));
        }
    }

    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    

}
