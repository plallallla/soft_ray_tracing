#pragma once

#include "HitTable.hpp"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include <iostream>

class Sphere : public HitTable
{
    glm::vec3 _center;
    float _radius;
public:
    Sphere(glm::vec3 center, float radius) : _center{center}, _radius{radius} {}
    // virtual bool hit(const Ray& r, Interval interval, HitRecord& record) const override
    // {
    //     glm::vec3 oc = _center - r.origin();
    //     auto dir = r.direction();
    //     auto a = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    //     auto h = glm::dot(dir, oc);
    //     auto c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - _radius * _radius;
    //     auto discriminant = h*h - a*c;
    //     if (discriminant < 0) return false;
    //     auto sqrtd = std::sqrt(discriminant);
    //     auto root = (h - sqrtd) / a;
    //     if (!(interval.surrounds(root)))
    //     {
    //         root = (h + sqrtd) / a;
    //         if (!(interval.surrounds(root))) return false;
    //     }
    //     record._t = root;
    //     record._point = r.at(root);
    //     glm::vec3 outward_normal = (record._point - _center) / _radius;
    //     record.set_face_normal(r, outward_normal);
    //     return true;
    // }   
    virtual bool hit(const Ray& r, Interval interval, HitRecord& record) const override
    {
        glm::vec3 oc = r.origin() - _center;
        glm::vec3 rd = r.direction();
        float a = glm::dot(rd, rd);
        float half_b = glm::dot(oc, rd);
        float c = glm::dot(oc, oc) - _radius * _radius;
        float discriminant = half_b * half_b - a * c;
        if (discriminant < 0.0f) return false;
        // 计算最近的 t（先尝试较小的根）
        float sqrtd = glm::sqrt(discriminant);
        float root = (-half_b - sqrtd) / a;
        // 如果不在 [min_t, max_t] 范围内，尝试另一个根
        if (!interval.surrounds(root)) root = (-half_b + sqrtd) / a;
        if (!interval.surrounds(root)) return false;
        // 填充 HitRecord
        record._t = root;
        record._point = r.at(root);
        glm::vec3 outward_normal = glm::normalize(record._point - _center); // 单位外法向
        record.set_face_normal(r, outward_normal);
        return true;
    }
};