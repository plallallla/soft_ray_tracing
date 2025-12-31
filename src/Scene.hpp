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

    auto depth = -6.f;
    auto length = -7.f;
    auto half_length = 3.5f;
    auto light_length = -2.5f;
    auto half_light_length = 1.25f;

    // world.add(std::make_shared<Quad>(
    //     glm::vec3(half_length, half_length, -6.f), glm::vec3(length, 0.f, 0.f), glm::vec3(0.f, length, 0.f), white));
    // world.add(std::make_shared<Quad>(
    //     glm::vec3(half_length, half_length, -6.f), glm::vec3(length, 0.f, 0.f), glm::vec3(0.f, 0.f, -length), white));  
    // world.add(std::make_shared<Quad>(
    //     glm::vec3(-half_length, -half_length, -6.f), glm::vec3(-length, 0.f, 0.f), glm::vec3(0.f, 0.f, -length), white));                
    // world.add(std::make_shared<Quad>(
    //     glm::vec3(half_length, half_length, -6.f), glm::vec3(0.f, length, 0.f), glm::vec3(0.f, 0.f, -length), red));         
    // world.add(std::make_shared<Quad>(
    //     glm::vec3(-half_length, -half_length, -6.f), glm::vec3(0.f, -length, 0.f), glm::vec3(0.f, 0.f, -length), green));         
    world.add(std::make_shared<Quad>(
        glm::vec3(-0.5f, 1e-4 - half_length, -3.75f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), light));  


    // shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    // box1 = make_shared<rotate_y>(box1, 15);
    // box1 = make_shared<translate>(box1, vec3(265,0,295));
    // world.add(box1);

    // shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    // box2 = make_shared<rotate_y>(box2, -18);
    // box2 = make_shared<translate>(box2, vec3(130,0,65));
    // world.add(box2);

    return world;

}