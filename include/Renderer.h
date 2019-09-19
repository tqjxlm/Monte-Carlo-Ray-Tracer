#pragma once

#include "Scene.h"

class Renderer {
public:

    Renderer(const Scene      & scene,
             const unsigned int MAX_DEPTH = 5);

    glm::vec3 getPixelColor(const Ray& ray) const
    {
        return traceRay(ray);
    }

private:

    glm::vec3 traceRay(const Ray        & ray,
                       const unsigned int DEPTH = 0) const;

private:

    const unsigned int maxDepth;
    const Scene& scene;
};
