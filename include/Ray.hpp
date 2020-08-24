#pragma once

#include <glm/glm.hpp>

struct Ray
{
    Ray(glm::vec3 from, glm::vec3 direction) :
        direction(direction),
        origin(from),
        direction_inv(1.0f / direction)
    {}

    Ray()
    {}

    glm::vec3 direction;
    glm::vec3 origin;
    glm::vec3 direction_inv;
};
