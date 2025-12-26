#pragma once
#include <glm/common.hpp>
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

class Dielectric : public Material
{
    float _refraction_index;// 介绍与空气折射率比值

public:
    Dielectric(float refraction_index) : _refraction_index(refraction_index) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    {
        glm::vec3 dir;
        float eta_ratio = !record._is_front ? _refraction_index : 1.f / _refraction_index;
        float cosion = glm::dot(-ray_in.direction(), record._normal);
        bool has_refract = refract_prediction(eta_ratio, cosion);
        float f0 = pow((1.f - eta_ratio) / (1.f + eta_ratio), 2);
        float reflect = schlick_approximation_fresnel(f0, cosion);
        if (!has_refract || reflect > RANDOM.get_float(0, 1)) dir = glm::normalize(glm::reflect(ray_in.direction(), record._normal));
        else dir = glm::normalize(glm::refract(ray_in.direction(), record._normal, eta_ratio)); 
        return ScatterResult{ true, {1.f, 1.f, 1.f}, {record._point, dir } };
    }

    // static float reflectance(float cosine, float refraction_index) {
    //     // Use Schlick's approximation for reflectance.
    //     auto r0 = (1 - refraction_index) / (1 + refraction_index);
    //     r0 = r0*r0;
    //     return r0 + (1-r0)*std::pow((1 - cosine),5);
    // }
    // virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    // {
    //     float ri = record._is_front ? (1.0/_refraction_index) : _refraction_index;

    //     glm::vec3 unit_direction = glm::normalize(ray_in.direction());
    //     float cos_theta = std::fmin(dot(-unit_direction, record._normal), 1.0);
    //     float sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

    //     bool cannot_refract = ri * sin_theta > 1.0;
    //     glm::vec3 dir;

    //     if (cannot_refract || reflectance(cos_theta, ri) > RANDOM.get_float(0, 1))
    //         dir = reflect(unit_direction, record._normal);
    //     else
    //         dir = refract(unit_direction, record._normal, ri);

    //     return ScatterResult{ true, {1.f, 1.f, 1.f}, {record._point, dir } };
    // }    
};