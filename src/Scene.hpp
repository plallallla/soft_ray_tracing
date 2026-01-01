#pragma once

#include "HitTable.hpp"
#include "Material.hpp"
#include "Transform.hpp"

inline HitTableList cornell_box()
{
    HitTableList world;
    auto red = std::make_shared<Lambertian>(glm::vec3(.65f, .05f, .05f));
    auto white = std::make_shared<Lambertian>(glm::vec3(.73f, .73f, .73f));
    auto green = std::make_shared<Lambertian>(glm::vec3(.12f, .45f, .15f));
    auto light = std::make_shared<DiffuseLight>(glm::vec3(15.f, 15.f, 15.f));

    auto depth = -15.f;
    auto length = -7.f;
    auto half_length = 3.5f;
    auto light_length = -2.5f;
    auto half_light_length = 1.25f;
    auto light_depth = -10.f;

    world.add(std::make_shared<Quad>(
        glm::vec3(half_length, half_length, depth), glm::vec3(length, 0.f, 0.f), glm::vec3(0.f, length, 0.f), white));
    world.add(std::make_shared<Quad>(
        glm::vec3(half_length, half_length, depth), glm::vec3(length, 0.f, 0.f), glm::vec3(0.f, 0.f, -length), white));  
    world.add(std::make_shared<Quad>(
        glm::vec3(-half_length, -half_length, depth), glm::vec3(-length, 0.f, 0.f), glm::vec3(0.f, 0.f, -length), white));                
    world.add(std::make_shared<Quad>(
        glm::vec3(half_length, half_length, depth), glm::vec3(0.f, length, 0.f), glm::vec3(0.f, 0.f, -length), red));         
    world.add(std::make_shared<Quad>(
        glm::vec3(-half_length, -half_length, depth), glm::vec3(0.f, -length, 0.f), glm::vec3(0.f, 0.f, -length), green));         
    world.add(std::make_shared<Quad>(
        glm::vec3(-0.5f, 1e-4 - half_length, light_depth), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), light));  

    float w1 = 1.65f;        
    auto box1 = create_box(w1, w1 * 2, w1, white);
    box1 = transform<RotateY>(box1, 15.f);
    box1 = transform<Translate>(box1, glm::vec3(-1.3f, half_length - w1, -11.4f));
    world.add(box1);

    float w2 = 1.65f;        
    auto box2 = create_box(w2, w2, w2, white);
    box2 = transform<RotateY>(box2, -18.f);
    box2 = transform<Translate>(box2, glm::vec3(1.3f, half_length - w2 / 2, -10.4f));
    world.add(box2);    

    return world;

}