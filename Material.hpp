#pragma once
#include "Ray.hpp"
#include "HitTable.hpp"
#include "tgaimage.hpp"
#include <glm/geometric.hpp>

struct ScatterResult
{
    operator bool() { return _has_scatter; }
    bool _has_scatter = false;
    TGAColor _color;
    Ray _ray;
};

class Material
{
public:
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const = 0;
};

class Metal
{
    TGAColor _albedo;
    float _fuzz;
public:
    Metal(const TGAColor& albedo, double fuzz) : _albedo(albedo), _fuzz(fuzz < 1.f ? fuzz : 1.f) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const
    {
        glm::vec3 reflected = glm::reflect(ray_in.direction(), record._normal);
        
    }

};