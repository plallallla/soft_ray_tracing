#pragma once

#include "HitTable.hpp"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

class Sphere : public HitTable
{
    glm::vec3 _center;
    float _radius;
    AABB _box;

public:
    Sphere(glm::vec3 center, float radius) : _center{center}, _radius{radius} 
    {
        glm::vec3 r = glm::vec3(radius);
        _box.set(center - r, center + r);
    }
    MaterialPtr _material;
    
    virtual bool hit(Ray& r, HitRecord& record) override
    {
        glm::vec3 orign = r.origin() - _center;
        float a = glm::dot(r.direction(), r.direction());
        float b = 2.f * glm::dot(orign, r.direction());
        float c = glm::dot(orign, orign) - _radius * _radius;
        float delta = b * b - 4 * a * c;
        if (delta < 0) return false;
        float root = (-1.f * b - sqrt(delta)) / (2.f * a);
        if (!r.valid_t(root)) root = (-1.f * b + sqrt(delta)) / (2.f * a);
        if (!r.valid_t(root)) return false;
        r.update_t_max(root);
        record._t = root;
        record._point = r.at(record._t);
        record.set_face_normal(r, record._point - _center);
        record._material = _material;
        return true;
    }

    virtual AABB get_aabb() const override
    {
        return _box;
    }
};