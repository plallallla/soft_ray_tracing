#pragma once

#include <glm/glm.hpp>
#include "Interval.hpp"

class Ray
{
    glm::vec3 _origin;
    glm::vec3 _direction; // 方向单位向量
    Interval _t_range{ .001f, Interval::f_max };
public:
    Ray() = default;
    Ray(const glm::vec3& origin, const glm::vec3& direction) : _origin{origin}, _direction{direction} {}
    glm::vec3 at(float t) const { return _origin + t * _direction; }
    glm::vec3 origin() const { return _origin; }
    glm::vec3 direction() const { return _direction; }
    void update_t_max(float t) { _t_range._max = t; }
    bool valid_t(float t) const { return _t_range.surrounds(t); }
};