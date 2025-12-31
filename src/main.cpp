#include <chrono>
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

    auto t1 = std::chrono::high_resolution_clock::now();
    camera.render(framebuffer, world);
    auto t2 = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    std::cout << "Rendering time: " << std::fixed << std::setprecision(3) << duration << " seconds" << std::endl;
    
    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    
}

