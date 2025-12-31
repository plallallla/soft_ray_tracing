#pragma once
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

#include "AABB.hpp"
#include "Interval.hpp"

class Material;
using MaterialPtr = std::shared_ptr<Material>;

struct HitRecord
{
    glm::vec3 _point;
    glm::vec3 _normal;
    glm::vec2 _uv;
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
protected:
    AABB _box;
public:    
    virtual bool hit(Ray& r, HitRecord& record) = 0;
    virtual AABB get_aabb() const { return _box; }
};
using HitTablePtr = std::shared_ptr<HitTable>;
using HitTablePtrs = std::vector<HitTablePtr>;

class HitTableList : public HitTable
{
    HitTablePtrs _list;
public:
    void add(const HitTablePtr& object)
    {
        _list.push_back(object);
        _box = _box + object->get_aabb();
    }

    void add(const HitTablePtrs& objects)
    {
        _list.insert(_list.end(), objects.begin(), objects.end());
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

};

class Translate : public HitTable
{
    HitTablePtr _object;
    glm::vec3 _offset;
public:
    Translate(const HitTablePtr& object, const glm::vec3& offset) : _object{object}, _offset{offset}
    {
        _box = object->get_aabb() + offset;
    }    
    virtual bool hit(Ray& r, HitRecord& record) override
    {
        Ray offset_r{r.origin() - _offset, r.direction()};
        if (!_object->hit(offset_r, record)) return false;
        record._point += _offset;
        return true;
    }
};

class RotateY : public HitTable
{
    HitTablePtr _object;
    float _sin_theta;
    float _cos_theta;
    bool _has_box;

public:
    RotateY(const HitTablePtr& object, float angle_degrees) : _object(object)
    {
        auto radians = glm::radians(angle_degrees);
        _sin_theta = std::sin(radians);
        _cos_theta = std::cos(radians);

        // 获取原始包围盒
        auto bbox = _object->get_aabb();

        // 计算旋转后的包围盒：遍历8个顶点
        glm::vec3 min_p( std::numeric_limits<float>::max());
        glm::vec3 max_p(-std::numeric_limits<float>::max());

        for (int i = 0; i < 2; ++i) 
        {
            for (int j = 0; j < 2; ++j) 
            {
                for (int k = 0; k < 2; ++k) 
                {
                    float x = i ? bbox.get_slab_x()._max : bbox.get_slab_x()._min;
                    float y = j ? bbox.get_slab_y()._max : bbox.get_slab_y()._min;
                    float z = k ? bbox.get_slab_z()._max : bbox.get_slab_z()._min;
                    // 绕 Y 轴旋转该点（逆时针，右手定则）
                    float new_x =  _cos_theta * x + _sin_theta * z;
                    float new_z = -_sin_theta * x + _cos_theta * z;
                    glm::vec3 rotated_point(new_x, y, new_z);
                    min_p = glm::min(min_p, rotated_point);
                    max_p = glm::max(max_p, rotated_point);
                }
            }
        }
        _box = AABB{min_p, max_p};
    }

    virtual bool hit(Ray& r, HitRecord& record) override
    {
        // 将光线从世界空间变换到物体空间
        // 绕 Y 轴的逆旋转 = 转置（因为是正交矩阵）= 用 -theta 旋转
        // 即：x' =  cos * x + sin * z
        //      z' = -sin * x + cos * z   ← 这是正向旋转
        // 所以逆旋转是：
        //      x_obj =  cos * x_world - sin * z_world
        //      z_obj =  sin * x_world + cos * z_world
        glm::vec3 origin = r.origin();
        glm::vec3 direction = r.direction();

        glm::vec3 new_origin
        {
            _cos_theta * origin.x - _sin_theta * origin.z,
            origin.y,
            _sin_theta * origin.x + _cos_theta * origin.z
        };

        glm::vec3 new_direction
        {
            _cos_theta * direction.x - _sin_theta * direction.z,
            direction.y,
            _sin_theta * direction.x + _cos_theta * direction.z
        };
        Ray rotated_ray(new_origin, new_direction);
        if (!_object->hit(rotated_ray, record)) return false;
        // 将命中点和法线从物体空间变换回世界空间
        glm::vec3 p = record._point;
        glm::vec3 normal = record._normal;
        // 正向旋转（theta）：世界 = R * 物体
        record._point = glm::vec3
        {
            _cos_theta * p.x + _sin_theta * p.z,
            p.y,
            -_sin_theta * p.x + _cos_theta * p.z
        };
        record._normal = glm::vec3
        {
            _cos_theta * normal.x + _sin_theta * normal.z,
            normal.y,
            -_sin_theta * normal.x + _cos_theta * normal.z
        };
        record._normal = glm::normalize(record._normal);
        return true;
    }
};

class Sphere : public HitTable
{
    glm::vec3 _center;
    float _radius;

    static glm::vec2 get_sphere_uv(const glm::vec3& p) 
    {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
        auto theta = std::acos(-p.y);
        auto phi = std::atan2(-p.z, p.x) + pi;
        return glm::vec2{ phi / (2.f * pi), theta / pi };
    }    

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
        auto outer_vec = record._point - _center;
        record.set_face_normal(r, outer_vec);
        record._uv = get_sphere_uv(outer_vec);
        record._material = _material;
        return true;
    }
};

class Quad : public HitTable
{
    glm::vec3 _Q;
    glm::vec3 _u;
    glm::vec3 _v;
    glm::vec3 _w;
    float _D;
    glm::vec3 _normal;
public:
    MaterialPtr _material;
    Quad(const glm::vec3& Q, const glm::vec3& u, const glm::vec3& v) : _Q{Q}, _u {u}, _v{v}
    {
        glm::vec3 n = glm::cross(u, v);
        _normal = glm::normalize(n);
        _D = glm::dot(_normal, Q);
        _w = n / glm::dot(n, n);
        _box = AABB{Q, Q + u + v} + AABB{Q + u, Q + v};
    }

    bool is_interior(float a, float b, HitRecord& rec) const 
    {
        static const Interval unit_interval{ 0.f, 1.f };
        if (!unit_interval.contains(a) || !unit_interval.contains(b)) return false;
        rec._uv = { a, b };
        return true;
    }    

    virtual bool hit(Ray& r, HitRecord& record) override
    {
        // 计算发现在光源方向的投影
        auto denom = glm::dot(_normal, r.direction());
        // 平行必不相交
        if (std::fabs(denom) < 1e-8) return false;
        // 计算交点
        auto t = (_D - glm::dot(_normal, r.origin())) / denom;
        // 非法交点说明不相交
        if (!r.valid_t(t)) return false;
        // 计算平面内的局部坐标
        glm::vec3 P = r.at(t);
        glm::vec3 P_Q = P - _Q;
        float alpha = glm::dot(_w, glm::cross(P_Q, _v));// α = _w · ((P - _Q) × _v)
        float beta = glm::dot(_w, glm::cross(_u, P_Q));// β = _w · (_u × (P - _Q))
        // 非法坐标 说明这一点值不在区间内
        if (!is_interior(alpha, beta, record)) return false;
        // 更新命中点
        r.update_t_max(t);
        record._point = P;
        record._material = _material;
        record.set_face_normal(r, _normal);
        return true;
    }    
};

inline HitTableList CreateBox(uint length, uint width, uint height, MaterialPtr material)
{
    HitTableList box;
    glm::vec3 p{ length / 2.f, width / 2.f, height / 2.f };
    glm::vec3 x{ length, 0.f, 0.f };
    glm::vec3 y{ 0.f, width, 0.f };
    glm::vec3 z{ 0.f, 0.f, height };
    std::vector<HitTablePtr> quads;
    quads.push_back(std::make_shared<Quad>(p, x, y));
    quads.push_back(std::make_shared<Quad>(p, x, z));
    quads.push_back(std::make_shared<Quad>(p, z, y));
    p *= -1;
    x *= -1;
    y *= -1;
    z *= -1;
    quads.push_back(std::make_shared<Quad>(p, x, y));
    quads.push_back(std::make_shared<Quad>(p, x, z));
    quads.push_back(std::make_shared<Quad>(p, z, y));
    for (auto& q : quads) box.add(q);
    return box;
}