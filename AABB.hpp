#pragma once
#include <array>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include "Interval.hpp"
#include "Ray.hpp"
#include "Utility.hpp"

class AABB
{
    Interval _slab_x;
    Interval _slab_y;
    Interval _slab_z;

    // 计算光线进入与离开某个slab的时间
    void slab_hit(const Interval& slab_range, Interval& t_range, float ori, float dir)
    {
        if (std::abs(dir) < epsilon)// 光线平行于该轴的两个平面
        {
            if (slab_range.contains(ori)) return;// 起点在slab内保持t_range不变
            t_range._max = 0.0f;// 起点不在slab内必定与此AABB无交点
            t_range._min = 1.0f;
            return;
        }        
        float t0 = (slab_range._min - ori) / dir;
        float t1 = (slab_range._max - ori) / dir;
        if (t1 < t0) std::swap(t0, t1);
        t_range._min = fmax(t0, t_range._min);
        t_range._max = fmin(t1, t_range._max);
    }

public:
    AABB() = default;
    AABB(const std::array<Interval, 3>& ranges)
    {
        set(ranges);
    }

    inline Interval get_slab_x() const { return _slab_x; }
    inline Interval get_slab_y() const { return _slab_y; }
    inline Interval get_slab_z() const { return _slab_z; }

    void set(const std::array<Interval, 3>& ranges)
    {
        _slab_x = ranges[0];
        _slab_y = ranges[1];
        _slab_z = ranges[2];
        if (_slab_x.length() < 1e-4f) _slab_x.expand(1e-4f / 2.f);
        if (_slab_y.length() < 1e-4f) _slab_y.expand(1e-4f / 2.f);
        if (_slab_z.length() < 1e-4f) _slab_z.expand(1e-4f / 2.f);
    }

    void set(glm::vec3 p1, glm::vec3 p2)
    {
        set(std::array<Interval, 3>{ Interval{p1.x,p2.x}, Interval{p1.y,p2.y}, Interval{p1.z,p2.z} });
    }

    // Slab Method
    bool hit(Ray& r)
    {
        const auto& dir = r.direction();
        const auto& orig = r.origin();
        Interval t_range = r.get_t_range();
        // 计算 x 轴
        slab_hit(_slab_x, t_range, orig.x, dir.x);
        if (t_range._min >= t_range._max) return false;
        // 计算 y 轴
        slab_hit(_slab_y, t_range, orig.y, dir.y);
        if (t_range._min >= t_range._max) return false;
        // 计算 z 轴
        slab_hit(_slab_z, t_range, orig.z, dir.z);
        return t_range._min < t_range._max;
    }

};

inline AABB operator+(const AABB& box, const glm::vec3& offset) 
{
    return std::array<Interval, 3>
    {
        box.get_slab_x() + offset.x, 
        box.get_slab_y() + offset.y, 
        box.get_slab_z() + offset.z
    };
}

inline AABB operator+(const glm::vec3& offset, const AABB& box) 
{
    return std::array<Interval, 3>
    {
        box.get_slab_x() + offset.x, 
        box.get_slab_y() + offset.y, 
        box.get_slab_z() + offset.z
    };
}