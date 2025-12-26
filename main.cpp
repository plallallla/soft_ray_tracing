#include <csetjmp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include "Material.hpp"
#include "tgaimage.hpp"

#include "Camera.hpp"
#include "Sphere.hpp"

int main(int, char**)
{
    HitTableList world;
    

    Lambertian ground_mat{ {.8f, .8f, .0f} };
    Lambertian l{ {.1f, .2f, .5f} };
    Metal m{ { .8f, .6f, .2f }, 0.2 };
    Dielectric d{ 1.5f };


    Sphere ground{glm::vec3{0.f, 100.5f, -1.5f}, 100.f};
    ground._material = std::make_shared<Lambertian>(ground_mat);
    world.add(std::make_shared<Sphere>(ground));

    Sphere mid{glm::vec3{0.f, 0.f, -1.5f}, .5f};
    mid._material = std::make_shared<Lambertian>(l);    
    world.add(std::make_shared<Sphere>(mid));

    Sphere left{glm::vec3{-1.f, 0.f, -1.5f}, .5f};
    left._material = std::make_shared<Dielectric>(d);
    world.add(std::make_shared<Sphere>(left));

    Sphere right{glm::vec3{1.f, 0.f, -1.5f}, .5f};
    right._material = std::make_shared<Metal>(m);
    world.add(std::make_shared<Sphere>(right));

    Camera camera;
    TGAImage framebuffer(camera.get_image_width(), camera.get_image_height(), TGAImage::RGB);
    camera.render(framebuffer, world);
    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    

}

