#pragma once
#include <__math/roots.h>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include "Ray.hpp"
#include "Utility.hpp"
#include "HitTable.hpp"
#include "Texture.hpp"

struct ScatterResult
{
    explicit operator bool() { return _has_scatter; }
    bool _has_scatter = false;
    glm::vec3 _attenuation;
    Ray _scattered_ray;
};

class Material
{
public:
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const { return { false }; };
    virtual glm::vec3 emitted(const glm::vec2 uv, const glm::vec3& p) const { return glm::vec3{ 0.f, 0.f, 0.f}; };
};
using MaterialPtr = std::shared_ptr<Material>;

class Lambertian : public Material
{
    glm::vec3 _albedo;
public:    
    Lambertian(const glm::vec3& albedo) : _albedo(albedo) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    {
        return { true, _albedo, {record._point, RANDOM.cosine_weighted_random_hemisphere(record._normal)} };
    }
};

class Metal : public Material
{
    glm::vec3 _albedo;
    float _fuzz;
public:
    Metal(const glm::vec3& albedo, float fuzz) : _albedo(albedo), _fuzz(fuzz < 1.f ? fuzz : 1.f) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    {
        glm::vec3 reflected = glm::reflect(ray_in.direction(), record._normal);
        reflected = glm::normalize(reflected + _fuzz * RANDOM.get_unit_vec3());
        return ScatterResult{ true, _albedo, { record._point, reflected }};
    }

};

class Dielectric : public Material
{
    float _refraction_index;// 介质与空气折射率比值
public:
    Dielectric(float refraction_index) : _refraction_index(refraction_index) {}
    virtual ScatterResult scatter(const Ray& ray_in, const HitRecord& record) const override
    {
        glm::vec3 I = glm::normalize(ray_in.direction());
        // 考虑光线方向是 空气->介质 or 介质->空气
        float eta_ratio = record._is_front ? (1.f / _refraction_index) : _refraction_index;
        // 计算夹角
        float cos_theta = glm::dot(-I, record._normal);
        // 根据斯涅尔定律计算能否发生折射
        bool has_refract = refract_prediction(eta_ratio, cos_theta);
        // 菲涅耳系数
        float f0 = pow((1.f - eta_ratio) / (1.f + eta_ratio), 2);
        // 计算真实反射率
        float reflect_prob = schlick_approximation_fresnel(f0, cos_theta);
        // 判断是发生反射还是折射并计算结果
        glm::vec3 dir = !has_refract && reflect_prob > RANDOM.get_float(0, 1) ? 
        glm::reflect(I, record._normal) : glm::refract(I, record._normal, eta_ratio);
        return ScatterResult{ true, {1.f, 1.f, 1.f}, { record._point, dir } };
    }
    
};

class DiffuseLight : public Material
{
private:
    TexturePtr _texture;
public:
    DiffuseLight(TexturePtr texture) : _texture{texture} {}
    DiffuseLight(const glm::vec3& color) : _texture{std::make_shared<SolidColor>(color)} {}
    virtual glm::vec3 emitted(const glm::vec2 uv, const glm::vec3& p) const override 
    {
        return _texture->value(uv, p);
    }
};