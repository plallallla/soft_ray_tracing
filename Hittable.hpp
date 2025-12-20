#pragma once
#include "Ray.hpp"
#include "Interval.hpp"

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

struct HitRecord
{
    glm::vec3 _point;
    glm::vec3 _normal;
    float _t;
    bool _is_front;

    void set_face_normal(const Ray& r, const glm::vec3& outward_normal)
    {
        _is_front = glm::dot(r.direction(), outward_normal) > 0;
        _normal = _is_front ? outward_normal : -1.f * outward_normal;
    }

};

class HitTable
{
public:    
    virtual bool hit(const Ray& r, Interval interval, HitRecord& record) const = 0;
};
using ht_ptr = std::shared_ptr<HitTable>;

class HitTableList : public HitTable
{
    std::vector<ht_ptr> _list;
public:
    void add(const ht_ptr& object)
    {
        _list.push_back(object);
    }
    virtual bool hit(const Ray& r, Interval range, HitRecord& record) const override
    {
        HitRecord temp_rec;
        bool hit_anything = false;
        auto closest_so_far = range.get_max();

        for (const auto& obj : _list) 
        {
            if (obj->hit(r, Interval{0.f, closest_so_far}, temp_rec)) 
            {
                hit_anything = true;
                closest_so_far = temp_rec._t;
                record = temp_rec;
            }
        }
        return hit_anything;
    }
};