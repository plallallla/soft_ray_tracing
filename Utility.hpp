#pragma once
#include <glm/geometric.hpp>
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
public:    
    float get_float(float min, float max)
    {
        std::uniform_real_distribution<float> dis(min, max);
        return dis(_gen);
    }
    // 拒绝采样法（Rejection Sampling）生成单位球面上的均匀随机方向
    glm::vec3 get_unit_vec3()
    {
        while (true) 
        {
            glm::vec3 p
            {
                get_float(-1.0f, 1.0f),
                get_float(-1.0f, 1.0f),
                get_float(-1.0f, 1.0f)
            };
            float len2 = glm::dot(p, p);
            if (len2 > 0.0f && len2 < 1.0f) return glm::normalize(p);
        }
    }
    // TODO 改为逆变换采样
    glm::vec3 cosine_weighted_random_hemisphere(const glm::vec3& normal) 
    {
        do 
        {
            glm::vec3 vec
            {
                get_float(-1.f, 1.f),
                get_float(-1.f, 1.f),
                get_float(-1.f, 1.f),
            };
            if (glm::dot(vec, vec) < 1.f) return glm::normalize(normal + vec);
        }
        while (true);
    }    
    
private:    
    SINGLETON(Random);
    std::random_device _rd;
    std::mt19937 _gen;    
};

inline bool is_zero_vec(const glm::vec3& vec)
{
    return std::fabs(vec.x) < 1e-8 && std::fabs(vec.y) < 1e-8 && std::fabs(vec.z) < 1e-8;
}

inline float schlick_approximation_fresnel(float f0, float cos_theta)
{
    return f0 + (1.f - f0) * std::pow(1.f - cos_theta, 5.f);
}

inline bool refract_prediction(float eta_ration, float cosine)
{
    return eta_ration * eta_ration * (1 - cosine * cosine) < 1;
}