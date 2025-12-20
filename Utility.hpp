#pragma once
#include <glm/glm.hpp>
#include <random>

#define SINGLETON(ClassName) \
public: \
    static ClassName& getInstance() { \
        static ClassName instance; \
        return instance; \
    } \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete; \
private: \
    ClassName() = default; \
    \
    ~ClassName() = default; \

#define RANDOM Random::getInstance()
class Random
{
    float random_float(float min, float max)
    {
        std::uniform_real_distribution<float> dis(min, max);
        return dis(_gen);
    }

    // 拒绝采样法（Rejection Sampling）生成单位球面上的均匀随机方向
    glm::vec3 random_unit_vec3()
    {
        while (true) 
        {
            glm::vec3 p
            {
                random_float(-1.0f, 1.0f),
                random_float(-1.0f, 1.0f),
                random_float(-1.0f, 1.0f)
            };
            float len2 = glm::dot(p, p);
            if (len2 > 0.0f && len2 < 1.0f) return glm::normalize(p);
        }
    }

   // 生成单位半球上的随机方向（用于漫反射）
    glm::vec3 random_hemisphere_vec3(const glm::vec3& normal)
    {
        glm::vec3 in_unit_sphere = random_unit_vec3();
        if (glm::dot(in_unit_sphere, normal) > 0.0f) return in_unit_sphere;
        else return -in_unit_sphere;
    }
    
private:    
    SINGLETON(Random);
    std::random_device _rd;
    std::mt19937 _gen;    
};