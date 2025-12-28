#pragma once
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

#include "AABB.hpp"

class Material;
using MaterialPtr = std::shared_ptr<Material>;

struct HitRecord
{
    glm::vec3 _point;
    glm::vec3 _normal;
    MaterialPtr _material;
    float _t;
    bool _is_front;

    void set_face_normal(const Ray& r, const glm::vec3& outward_normal)
    {
        _is_front = glm::dot(r.direction(), outward_normal) < 0;
        _normal = _is_front ? outward_normal : -1.f * outward_normal;
        _normal = glm::normalize(_normal);
    }

};

class HitTable
{
public:    
    virtual bool hit(Ray& r, HitRecord& record) = 0;
    virtual AABB get_aabb() const = 0;
};
using HitTablePtr = std::shared_ptr<HitTable>;

class HitTableList : public HitTable
{
    std::vector<HitTablePtr> _list;
    AABB _box;
public:
    void add(const HitTablePtr& object)
    {
        _list.push_back(object);
    }
    virtual bool hit(Ray& r, HitRecord& record) override
    {
        bool hit_anything = false;
        for (const auto& obj : _list) 
        {
            if (obj->hit(r, record)) 
            {
                hit_anything = true;
            }
        }
        return hit_anything;
    }

    virtual AABB get_aabb() const override
    {
        return _box;
    }

};