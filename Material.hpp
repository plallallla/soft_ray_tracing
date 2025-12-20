#pragma once
#include <glm/geometric.hpp>
#include <memory>
#include "Ray.hpp"
#include "Utility.hpp"
#include "HitTable.hpp"

struct ScatterResult
{
    operator bool() { return _has_scatter; }
    bool _has_scatter = false;
    glm::vec3 _attenuation;
    Ray _scattered_ray;
};

class Material
{
public:
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const = 0;
};
using MaterialPtr = std::shared_ptr<Material>;

class Lambertian : public Material
{
    glm::vec3 _albedo;
public:    
    Lambertian(const glm::vec3& albedo) : _albedo(albedo) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    {
        ScatterResult scatter_result;
        auto scatter_direction = record._normal + RANDOM.get_unit_vec3();
        if (is_zero_vec(scatter_direction)) scatter_direction = record._normal;
        scatter_result._scattered_ray = Ray{record._point, scatter_direction};
        scatter_result._attenuation = _albedo;
        return { true, _albedo, Ray{record._point, scatter_direction} };
    }
};

class Metal
{
    glm::vec3 _albedo;
    float _fuzz;
public:
    Metal(const glm::vec3& albedo, double fuzz) : _albedo(albedo), _fuzz(fuzz < 1.f ? fuzz : 1.f) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const
    {
        glm::vec3 reflected = glm::reflect(ray_in.direction(), record._normal);
        return ScatterResult{};
    }

};