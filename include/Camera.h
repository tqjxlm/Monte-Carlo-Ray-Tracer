#pragma once

#include <vector>
#include <chrono>

#include "Scene.h"
#include "Renderer.h"

struct HumanTime {
    long long h, m, s;
};

class Camera {
public:

    Camera(const unsigned int width  = 1000,
           const unsigned int height = 1000);

    HumanTime   render(const Scene& scene,
                Renderer& renderer,
                const unsigned int RAYS_PER_PIXEL = 1024,
                const glm::vec3 eye = glm::vec3(-7, 0, 0),
                glm::vec3 direction = glm::vec3(1, 0, 0),
                glm::vec3 up        = glm::vec3(0, 0, 1));

    bool writeImageTGA(const std::string& path = "output.tga") const;

private:

    void createImage();

    void logProgress();

private:

    unsigned int width;
    unsigned int height;

    // Pixel containers
    std::vector<std::vector<glm::vec3> >pixels;
    std::vector<std::vector<glm::u8vec3> >discretizedPixels;

    // Progress
    int totalLines;
    int currentLineNum;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastLog;
};
