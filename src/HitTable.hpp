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
    MaterialPtr _material;
public:
    Quad(const glm::vec3& Q, const glm::vec3& u, const glm::vec3& v, MaterialPtr material = nullptr) 
    : _Q{Q}, _u {u}, _v{v}, _material{material}
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

inline HitTablePtr create_box(float x_len, float y_height, float z_depth, MaterialPtr material)
{
    HitTableList box;
    float hx = x_len / 2.0f;
    float hy = y_height / 2.0f;
    float hz = z_depth / 2.0f;

    // 左面 (x = -hx), 法向量 (-1, 0, 0)
    box.add(std::make_shared<Quad>(
        glm::vec3(-hx, -hy, -hz),
        glm::vec3(0, 0, z_depth),
        glm::vec3(0, y_height, 0),
        material
    ));

    // 右面 (x = +hx), 法向量 (+1, 0, 0)
    box.add(std::make_shared<Quad>(
        glm::vec3(hx, -hy, -hz),
        glm::vec3(0, y_height, 0),
        glm::vec3(0, 0, z_depth),
        material
    ));

    // 下面 (y = -hy), 法向量 (0, -1, 0)
    box.add(std::make_shared<Quad>(
        glm::vec3(-hx, -hy, -hz),
        glm::vec3(x_len, 0, 0),
        glm::vec3(0, 0, z_depth),
        material
    ));

    // 上面 (y = +hy), 法向量 (0, +1, 0)
    box.add(std::make_shared<Quad>(
        glm::vec3(-hx, hy, -hz),
        glm::vec3(0, 0, z_depth),
        glm::vec3(x_len, 0, 0),
        material
    ));

    // 后面 (z = -hz), 法向量 (0, 0, -1)
    box.add(std::make_shared<Quad>(
        glm::vec3(-hx, -hy, -hz),
        glm::vec3(x_len, 0, 0),
        glm::vec3(0, y_height, 0),
        material
    ));

    // 前面 (z = +hz), 法向量 (0, 0, +1)
    box.add(std::make_shared<Quad>(
        glm::vec3(-hx, -hy, hz),
        glm::vec3(0, y_height, 0),
        glm::vec3(x_len, 0, 0),
        material
    ));

    return std::make_shared<HitTableList>(box);
}