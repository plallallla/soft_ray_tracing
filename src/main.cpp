#include <csetjmp>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include "tgaimage.hpp"
#include "Camera.hpp"
#include "Scene.hpp"

int main(int, char**)
{
    HitTableList world = cornell_box();
    Camera camera;
    TGAImage framebuffer(camera.get_image_width(), camera.get_image_height(), TGAImage::RGB);
    camera.render(framebuffer, world);
    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    
}

