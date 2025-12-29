#pragma once

#include "HitTable.hpp"
#include <vector>

class Object : public HitTable
{
    std::vector<glm::vec3> _vertex;
    std::vector<glm::vec3> _index;
};