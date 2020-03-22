#include <iostream>
#include <string>

#include <cxxopts.hpp>

#include "Camera.h"
#include "Renderer.h"
#include "Math.hpp"

enum SceneID
{
    SCENE01 = 1,
    SCENE02,
    SCENE03,
    SCENE04
};

// Load a predefined scene
void loadScene(Scene& scene, SceneID sceneID)
{
    switch (sceneID)
    {
    case (SCENE01):
        scene.addObj("resources/scene01/cube1.obj");
        scene.addObj("resources/scene01/cube2.obj");
        scene.addObj("resources/scene01/sphere1.obj");
        scene.addObj("resources/scene01/sphere2.obj");
        break;

    case (SCENE02):
        scene.addObj("resources/scene02/scene02.obj");
        break;

    case (SCENE03):
        scene.addObj("resources/scene03/cube1.obj");
        scene.addObj("resources/scene03/cube2.obj");
        scene.addObj("resources/scene03/sphere1.obj",
                     glm::vec3(1.5f, 1.0f, 1.5f), glm::vec3(),
                     glm::vec3(2.0f, 2.0f, 2.0f));
        scene.addObj("resources/scene03/sphere2.obj",
                     glm::vec3(2.5f, 7.0f,    -1.0f), glm::vec3(),
                     glm::vec3(2.0f, 2.0f, 2.0f));
        scene.addObj("resources/scene03/dragon.obj",
                     glm::vec3(0.0f, 0.0f,       0.0f),
                     glm::vec3(0.0f,    -150.0f, 0.0f),
                     glm::vec3(5.0f, 5.0f,       5.0f));
        break;

    case (SCENE04):
        scene.addObj("resources/scene04/cube1.obj");
        scene.addObj("resources/scene04/cube2.obj",
                     glm::vec3(0.0f, 0.0f, 4.0f));
        scene.addObj("resources/scene04/kitten.obj",
                     glm::vec3(-2.0f,  0.0f,  0.0f),
                     glm::vec3(0.0f,   0.0f,  0.0f),
                     glm::vec3(0.05f, 0.05f, 0.05f));
        scene.addObj("resources/scene04/bunny.obj",
                     glm::vec3(2.0f,  1.4f, 0.0f),
                     glm::vec3(0.0f, 30.0f, 0.0f),
                     glm::vec3(1.5f,  1.5f, 1.5f));
        scene.addObj("resources/scene04/sphere1.obj",
                     glm::vec3(-4.8f, 1.0f, 3.0f),
                     glm::vec3(0.0f,  0.0f, 0.0f),
                     glm::vec3(2.0f,  2.0f, 2.0f));
        break;

    default:
        std::cout << "Error: undefined scene ID: " << (int)sceneID << std::endl;
        return;
    }

    std::cout << "Scene " << (int)sceneID << " loaded." << std::endl;

    return;
}

void renderScene(const Scene& scene,
                 SceneID      sceneID,
                 Camera     & camera,
                 int          maxRayDepth,
                 int          samplePerPixel)
{
    Renderer* renderer = new Renderer(scene, maxRayDepth);
    glm::vec3 eye;
    glm::vec3 direction;
    const glm::vec3 up = glm::vec3(0, 1, 0);

    switch (sceneID)
    {
    case (SCENE02):
        eye       = glm::vec3(2, 7, 19);
        direction = glm::vec3(0, -0.3, -1);
        break;

    case (SCENE04):
        eye       = glm::vec3(0, 5, 17);
        direction = glm::vec3(0, 0, -1);
        break;

    default:
        eye       = glm::vec3(0, 5, 15);
        direction = glm::vec3(0, 0, -1);
    }

    camera.render(scene, *renderer, samplePerPixel, eye, direction, up);

    delete renderer;
}

int main(int argc, char** argv)
{
    // Get render settings
    cxxopts::Options options("PathTracer", "Monte-Carlo path tracer");

    options.add_options()
        ("s,scene", "Model scene ID (default 1)",
            cxxopts::value<unsigned int>()->default_value("1"))
        ("r,ray", "Sample ray number per pixel (default 4)",
            cxxopts::value<unsigned int>()->default_value("4"))
        ("d,depth", "Maximum trace depth (default 4)",
            cxxopts::value<unsigned int>()->default_value("4"))
        ("p,pixel", "Pixel resolution width & height (default 1024)",
            cxxopts::value<unsigned int>()->default_value("1024"));

    auto result = options.parse(argc, argv);

    const unsigned int width          = result["pixel"].as<unsigned int>();
    const unsigned int height         = result["pixel"].as<unsigned int>();
    const unsigned int samplePerPixel = result["ray"].as<unsigned int>();
    const unsigned int maxRayDepth    = result["depth"].as<unsigned int>();
    const SceneID predefinedScene     = static_cast<SceneID>(result["scene"].as<unsigned int>());

    // Create scene
    Scene scene;

    try
    {
        loadScene(scene, predefinedScene);
    }
    catch (...)
    {
        std::cout << "Scene loading failed." << std::endl;
        std::cout << "Press any key to exit.";
        std::cin.get();

        return 1;
    }

    scene.initialize();

    // Render scene
    Camera camera(width, height);
    renderScene(scene, predefinedScene, camera, maxRayDepth, samplePerPixel);

    // Write out
    std::string imageFileName = Math::currentDateTime() + ".tga";
    camera.writeImageTGA(imageFileName);
    std::cout << "Image saved to: " << imageFileName << "." << std::endl;

    // Finished
    std::cout << "Press any key to exit.";
    std::cin.get();

    return 0;
}
