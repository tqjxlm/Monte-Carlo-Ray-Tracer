#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include "Camera.h"
#include "Renderer.h"
#include "Math.hpp"

enum PredefinedScene
{
    SCENE01 = 1,
    SCENE02,
    SCENE03,
    SCENE04
};

// Load a predefined scene
bool  loadScene(Scene &scene, PredefinedScene predefinedScene)
{
    switch (predefinedScene)
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
                     glm::vec3(1.5f, 1.0f, 1.5f), glm::vec3(), glm::vec3(2.0f, 2.0f, 2.0f));
        scene.addObj("resources/scene03/sphere2.obj",
                     glm::vec3(2.5f, 7.0f, -1.0f), glm::vec3(), glm::vec3(2.0f, 2.0f, 2.0f));
        // scene.addObj("resources/scene03/sphere3.obj",
        // glm::vec3(-3.5f, 0.5f, 4.5f), glm::vec3(), glm::vec3(1.0f, 1.0f, 1.0f));
        scene.addObj("resources/scene03/dragon.obj",
                     glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -150.0f, 0.0f), glm::vec3(5.0f, 5.0f, 5.0f));
        break;
    case (SCENE04):
        scene.addObj("resources/scene04/cube1.obj");
        scene.addObj("resources/scene04/cube2.obj",
                     glm::vec3(0.0f, 0.0f, 4.0f));
        scene.addObj("resources/scene04/kitten.obj",
                     glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));
        scene.addObj("resources/scene04/bunny.obj",
                     glm::vec3(2.0f, 1.4f, 0.0f), glm::vec3(0.0f, 30.0f, 0.0f), glm::vec3(1.5f, 1.5f, 1.5f));
        scene.addObj("resources/scene04/sphere1.obj",
                     glm::vec3(-4.8f, 1.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
        break;
    default:

        return false;
    }

    std::cout << "Scene loaded." << std::endl;

    return true;
}

void  renderScene(const Scene &scene, PredefinedScene predefinedScene, Camera &camera, int MAX_RAY_DEPTH, int RAYS_PER_PIXEL)
{
    Renderer *renderer = new Renderer(scene, MAX_RAY_DEPTH);

    switch (predefinedScene)
    {
    case (SCENE02):
        camera.render(scene, *renderer, RAYS_PER_PIXEL,
                      glm::vec3(2, 7, 19),
                      glm::vec3(0, -0.3, -1),
                      glm::vec3(0, 1, 0)
                      );
        break;
    case (SCENE04):
        camera.render(scene, *renderer, RAYS_PER_PIXEL,
                      glm::vec3(0, 5, 17),
                      glm::vec3(0, 0, -1),
                      glm::vec3(0, 1, 0)
                      );
        break;
    default:
        camera.render(scene, *renderer, RAYS_PER_PIXEL,
                      glm::vec3(0, 5, 15),
                      glm::vec3(0, 0, -1),
                      glm::vec3(0, 1, 0)
                      );
    }

    delete renderer;
}

int  main()
{
    // Get render settings
    const unsigned int     PIXELS_W        = 1024;
    const unsigned int     PIXELS_H        = 1024;
    const unsigned int     RAYS_PER_PIXEL  = 128;
    const unsigned int     MAX_RAY_DEPTH   = 5;
    const PredefinedScene  predefinedScene = SCENE04;

    // Create scene
    Scene  scene;

    if (!loadScene(scene, predefinedScene))
    {
        return 1;
    }

    scene.initialize();

    // Render scene
    Camera  camera(PIXELS_W, PIXELS_H);
    renderScene(scene, predefinedScene, camera, MAX_RAY_DEPTH, RAYS_PER_PIXEL);

    // Write out
    std::string  imageFileName = Math::currentDateTime() + ".tga";
    camera.writeImageTGA(imageFileName);
    std::cout << "Image saved to: " << imageFileName << "." << std::endl;

    // Finished
    std::cout << "Press any key to exit.";
    std::cin.get();

    return 0;
}
