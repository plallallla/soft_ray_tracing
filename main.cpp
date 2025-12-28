#include <csetjmp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include "Material.hpp"
#include "Utility.hpp"
#include "tgaimage.hpp"

#include "Camera.hpp"
#include "Sphere.hpp"

int main(int, char**)
{
    HitTableList world;
    
    auto ground_mat = std::make_shared<Lambertian>(glm::vec3{.5f, .5f, .0f});
    auto l = std::make_shared<Lambertian>(glm::vec3{.1f, .2f, .5f});
    auto m = std::make_shared<Metal>(glm::vec3{ .8f, .6f, .2f }, 0.2);
    auto d = std::make_shared<Dielectric>(1.5f);

    Sphere ground{glm::vec3{0.f, 100.5f, -1.5f}, 100.f};
    ground._material = ground_mat;
    world.add(std::make_shared<Sphere>(ground));

    Sphere mid{glm::vec3{0.f, 0.f, -1.5f}, .5f};
    mid._material = l;
    world.add(std::make_shared<Sphere>(mid));

    Sphere left{glm::vec3{-1.f, 0.f, -1.5f}, .5f};
    left._material = d;
    world.add(std::make_shared<Sphere>(left));

    Sphere right{glm::vec3{1.f, 0.f, -1.5f}, .5f};
    right._material = m;
    world.add(std::make_shared<Sphere>(right));

    Camera camera;
    TGAImage framebuffer(camera.get_image_width(), camera.get_image_height(), TGAImage::RGB);
    camera.render(framebuffer, world);
    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    

}

