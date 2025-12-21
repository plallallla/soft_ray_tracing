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
    

    Lambertian gray_diffuse{glm::vec3(.2f,.3f,.4f)};
    Lambertian yellow_diffuse{glm::vec3(1.f,1.f,0.f)};
    Lambertian white_diffuse{glm::vec3(1.f,1.f,1.f)};

    Sphere ground{glm::vec3{0.f, 100.5f, -1.f}, 100.f};
    ground._material = std::make_shared<Lambertian>(yellow_diffuse);
    Sphere object{glm::vec3{0.f, 0.f, -1.f}, .5f};
    object._material = std::make_shared<Lambertian>(yellow_diffuse);
    
    world.add(std::make_shared<Sphere>(ground));
    world.add(std::make_shared<Sphere>(object));

    Camera camera;
    
    TGAImage framebuffer(camera.get_image_width(), camera.get_image_height(), TGAImage::RGB);



    camera.render(framebuffer, world);



    
    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    

}

