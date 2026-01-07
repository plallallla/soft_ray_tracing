#include <chrono>
#include <csetjmp>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include "tgaimage.hpp"
#include "Camera.hpp"
#include "Scene.hpp"

int main()
{
    Camera camera;
    TGAImage framebuffer(camera.get_image_width(), camera.get_image_height(), TGAImage::RGB);

    auto world = cornell_box();
    HitTablePtr node = std::make_shared<BVHnode>(world);
    HitTableList scene;
    scene.add(node);
    auto t1 = std::chrono::high_resolution_clock::now();
    // camera.render(framebuffer, world);
    camera.render(framebuffer, scene);
    auto t2 = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    std::cout << "Rendering time: " << std::fixed << std::setprecision(3) << duration << " seconds" << std::endl;
    
    framebuffer.write_tga_file("ray_trace.tga");
    system("open ray_trace.tga");    
}

