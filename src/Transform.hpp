#pragma once
#include "HitTable.hpp"

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


template<typename T>
constexpr bool is_allowed_transform_v
{
    std::disjunction_v
    <
        std::is_same<std::decay_t<T>, Translate>,
        std::is_same<std::decay_t<T>, RotateY>
    >
};

template<typename Transform, typename... Args>
HitTablePtr transform(const HitTablePtr& obj, Args&&... args)
{
    static_assert(is_allowed_transform_v<Transform>, "Transform type not allowed!");
    return std::make_shared<Transform>(obj, std::forward<Args>(args)...);
}