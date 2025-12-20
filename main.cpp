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
    

    Lambertian yellow_diffuse{glm::vec3(1.f,1.f,0.f)};

    // Sphere ground{glm::vec3{0.f, 100.f, 0.f}, 100.f};
    // world.add(std::make_shared<Sphere>(ground));

    Sphere object{glm::vec3{0.f, 0.f, -1.f}, .5f};
    object._material = std::make_shared<Lambertian>(yellow_diffuse);
    world.add(std::make_shared<Sphere>(object));

    Camera camera;
    camera.init();
    
    TGAImage framebuffer(camera.get_image_width(), camera.get_image_height(), TGAImage::RGB);
    camera.render(framebuffer, world);
    
    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    

}
