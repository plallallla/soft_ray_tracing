#pragma once

#include <glm/glm.hpp>

class Ray
{
    glm::vec3 _origin;
    glm::vec3 _direction;
public:
    Ray(const glm::vec3& origin, const glm::vec3& direction) : _origin{origin}, _direction{direction} {}
    glm::vec3 at(float t) const { return _origin + t * _direction; }
    glm::vec3 origin() const { return _origin; }
    glm::vec3 direction() const { return _direction; }
};