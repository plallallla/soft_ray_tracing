#pragma once
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

class HitTableList : public HitTable
{
    std::vector<HitTablePtr> _list;
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
    Quad(const glm::vec3& Q, const glm::vec3& u, const glm::vec3& v)
    {
        glm::vec3 n = glm::cross(u, v);
        _normal = glm::normalize(n);
        _D = dot(_normal, Q);
        _w = n / dot(n,n);

        AABB box1 = AABB{Q, Q + u + v};
        AABB box2 = AABB(Q + u, Q + v);
        _box = box1 + box2;
    }

    bool is_interior(float a, float b, HitRecord& rec) const 
    {
        Interval unit_interval{ 0., 1. };
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.
        if (!unit_interval.contains(a) || !unit_interval.contains(b)) return false;
        rec._uv = { a, b };
        return true;
    }    

    virtual bool hit(Ray& r, HitRecord& record) override
    {
        auto denom = glm::dot(_normal, r.direction());

        // No hit if the ray is parallel to the plane.
        if (std::fabs(denom) < 1e-8) return false;

        // Return false if the hit point parameter t is outside the ray interval.
        auto t = (_D - glm::dot(_normal, r.origin())) / denom;
        if (!r.valid_t(t)) return false;
            

        // Determine if the hit point lies within the planar shape using its plane coordinates.
        auto intersection = r.at(t);
        auto planar_hitpt_vector = intersection - _Q;
        auto alpha = dot(_w, cross(planar_hitpt_vector, _v));
        auto beta = dot(_w, cross(_u, planar_hitpt_vector));

        if (!is_interior(alpha, beta, record)) return false;

        // Ray hits the 2D shape; set the rest of the hit record and return true.
        r.update_t_max(t);
        record._point = intersection;
        record._material = _material;
        record.set_face_normal(r, _normal);

        return true;
    }    
};

// class Object : public HitTable
// {
//     std::vector<glm::vec3> _vertex;
//     std::vector<glm::vec3> _index;
// };