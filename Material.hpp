#pragma once
#include <glm/geometric.hpp>
#include <memory>
#include "Ray.hpp"
#include "Utility.hpp"
#include "HitTable.hpp"

struct ScatterResult
{
    explicit operator bool() { return _has_scatter; }
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
        return { true, _albedo, {record._point, RANDOM.cosine_weighted_random_hemisphere(record._normal)} };
    }
};

class Metal : public Material
{
    glm::vec3 _albedo;
    float _fuzz;
public:
    Metal(const glm::vec3& albedo, float fuzz) : _albedo(albedo), _fuzz(fuzz < 1.f ? fuzz : 1.f) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    {
        glm::vec3 reflected = glm::normalize(glm::reflect(ray_in.direction(), record._normal));
        reflected = glm::normalize(reflected) + _fuzz * RANDOM.get_unit_vec3();
        return ScatterResult{ true, _albedo, { record._point, glm::normalize(reflected) }};
    }

};

class Dielectric : Material
{
    float _refraction_index;
    static float schlick_reflectance(float cosine, float refraction_index)
    {
        auto r0 = (1.f - refraction_index) / (1.f + refraction_index);
        r0 *= r0;
        return r0 + (1.f - r0) * std::pow((1.f - cosine), 5);        
    }
public:
    Dielectric(float refraction_index) : _refraction_index(refraction_index) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    {
        float ri = record._is_front ? (1.f / _refraction_index) : _refraction_index;
        float cos_theta = std::fmin(dot(-1.f * ray_in.direction(), record._normal), 0.0);
        float sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);
        bool cannot_refract = ri * sin_theta > 1.0;
        glm::vec3 direction = cannot_refract || schlick_reflectance(cos_theta, ri) > RANDOM.get_float(0, 1) ?
        glm::reflect(ray_in.direction(), record._normal) : refract(ray_in.direction(), record._normal, ri);
        return ScatterResult{ true, { 1.f, 1.f, 1.f }, { record._point, glm::normalize(direction) }};
    }
};