#pragma once

#include <glm/glm.hpp>

struct Ray
{
    Ray(glm::vec3 from, glm::vec3 direction) :
        direction(direction),
        origin(from),
        direction_inv(1.0f / direction),
        sign{direction_inv.x < 0, direction_inv.y < 0, direction_inv.z < 0}
    {}

    Ray()
    {}

    glm::vec3 direction;
    glm::vec3 origin;
    glm::vec3 direction_inv;
    bool      sign[3];
};
