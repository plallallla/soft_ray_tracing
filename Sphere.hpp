#pragma once

#include "HitTable.hpp"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include <iostream>

class Sphere : public HitTable
{
public:
    glm::vec3 _center;
    float _radius;
    Sphere(glm::vec3 center, float radius) : _center{center}, _radius{radius} {}
    
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
        record._point = r.at(root);
        record.set_face_normal(r, r.at(root) - _center);
        return true;
    }
};