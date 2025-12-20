#pragma once

#include "HitTable.hpp"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

class Sphere : public HitTable
{
    glm::vec3 _center;
    float _radius;

public:
    Sphere(glm::vec3 center, float radius) : _center{center}, _radius{radius} {}
    MaterialPtr _material;
    
    virtual bool hit(const Ray& r, Interval interval, HitRecord& record) const override
    {
        glm::vec3 orign = r.origin() - _center;
        float a = glm::dot(r.direction(), r.direction());
        float b = 2.f * glm::dot(orign, r.direction());
        float c = glm::dot(orign, orign) - _radius * _radius;
        float delta = b * b - 4 * a * c;
        if (delta < 0) return false;
        float root = (-1.f * b - sqrt(delta)) / (2.f * a);
        if (!interval.surrounds(root)) root = (-1.f * b + sqrt(delta)) / (2.f * a);
        if (!interval.surrounds(root)) return false;
        record._t = root;
        record._point = r.at(record._t);
        // record.set_face_normal(r, record._point - _center);
        record._normal = record._point - _center;
        record._material = _material;
        return true;
    }
};