#pragma once
#include <algorithm>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include "HitTable.hpp"
#include "AABB.hpp"


static bool box_x_compare(const HitTablePtr a, const HitTablePtr b)
{
    return a->get_aabb().get_slab_x()._min < b->get_aabb().get_slab_x()._min;
}
static bool box_y_compare(const HitTablePtr a, const HitTablePtr b) 
{
    return a->get_aabb().get_slab_y()._min < b->get_aabb().get_slab_y()._min;
}
static bool box_z_compare(const HitTablePtr a, const HitTablePtr b) 
{
    return a->get_aabb().get_slab_z()._min < b->get_aabb().get_slab_z()._min;
}
using box_compare = bool(*)(const HitTablePtr, const HitTablePtr);

class BVHnode : public HitTable
{
    HitTablePtr _left;
    HitTablePtr _right;
    AABB _box;
    static std::unordered_map<AXIS, box_compare> _compare;
public:
    BVHnode(HitTablePtrs& objects) : BVHnode{objects, 0, objects.size() } {}
    BVHnode(HitTablePtrs& objects, size_t begin, size_t end)
    {
        for (int i = begin; i != end; i++)
        {
            _box = objects[i]->get_aabb() + _box;
        }
        auto span = end - begin;
        if (span == 1)
        {
            _left = objects[begin];
            _right = objects[begin];
        }
        else if (span == 2)
        {
            _left = objects[begin];
            _right = objects[begin + 1];  
        }
        else
        {
            std::sort(objects.begin() + begin, objects.begin() + end, _compare[_box.longest_axis()]);
            auto mid = begin + span / 2;
            _left = std::make_shared<BVHnode>(objects, begin, mid);
            _right = std::make_shared<BVHnode>(objects, mid, end);
        }
    }
    virtual bool hit(Ray& r, HitRecord& record) override
    {
        if (!_box.hit(r)) return false;
        return _left->hit(r, record) || _right->hit(r, record);
        // bool hit_left = _left->hit(r, record);
        // bool hit_anything = hit_left;
        // if (hit_left) r.update_t_max(record._t);
        // HitRecord right_record;
        // if (_right->hit(r, right_record)) 
        // {
        //     if (!hit_left || right_record._t < record._t) 
        //     {
        //         record = right_record;
        //     }
        //     hit_anything = true;
        // }
        // return hit_anything;
    }

    virtual AABB get_aabb() const override { return _box; }

};

inline std::unordered_map<AXIS, box_compare> BVHnode::_compare
{
    { AXIS::X_AXIS, box_x_compare },
    { AXIS::Y_AXIS, box_y_compare },
    { AXIS::Z_AXIS, box_z_compare }
};