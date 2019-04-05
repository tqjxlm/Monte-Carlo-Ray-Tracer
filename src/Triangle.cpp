#include "Triangle.h"

#include <iostream>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>

#include "Math.hpp"

Triangle::Triangle(glm::vec3 _v1, glm::vec3 _v2, glm::vec3 _v3, glm::vec3 _n1, glm::vec3 _n2, glm::vec3 _n3, int index):
    vertices{_v1, _v2, _v3}, normals{_n1, _n2, _n3}, edges{_v2 - _v1, _v3 - _v1}, meshIndex(index)
{
    const auto &v0 = vertices[0];
    const auto &v1 = vertices[1];
    const auto &v2 = vertices[2];

    v       = vertices[2] - vertices[0];
    u       = vertices[1] - vertices[0];
    uv      = glm::dot(u, v);
    vv      = glm::dot(v, v);
    uu      = glm::dot(u, u);
    fractor = uv * uv - uu * vv;

    if ((abs(glm::dot(_n1, _n2) - 1.0f) < std::numeric_limits<float>::min()) && (abs(glm::dot(_n2, _n3)) - 1.0f < std::numeric_limits<float>::min()))
    {
        faceNorm = glm::normalize((_n1 + _n2 + _n3) / 3.0f);
        flat     = true;
    }
    else if ((_n1 == glm::vec3()) || (_n2 == glm::vec3()) || (_n3 == glm::vec3()))
    {
        flat     = true;
        faceNorm = Math::calcNormal(_v1, _v2, _v3);
    }
    else
    {
        flat     = false;
        faceNorm = Math::calcNormal(_v1, _v2, _v3);

        for (auto &norm : normals)
        {
            if (glm::dot(faceNorm, norm) < 0.0f)
            {
                norm *= -1.0f;
            }
        }
    }
}

glm::vec3  Triangle::getRandomPositionOnSurface() const
{
    glm::vec3  v1                       = vertices[1] - vertices[0];
    glm::vec3  v2                       = vertices[2] - vertices[0];
    glm::vec3  randomRectanglePoint     = (rand() / (float)RAND_MAX) * v1 + (rand() / (float)RAND_MAX) * v2;
    glm::vec3  pointProjectedOnV1V2Line = glm::closestPointOnLine(randomRectanglePoint, v1, v2);

    // If its further to the random point than to the line point then we're outside the triangle
    if (glm::length(randomRectanglePoint) > glm::length(pointProjectedOnV1V2Line))
    {
        // Move random point inside triangle
        randomRectanglePoint += (pointProjectedOnV1V2Line - randomRectanglePoint) * 2.0f;
    }

    return vertices[0] + randomRectanglePoint;
}

bool  Triangle::rayIntersection(const Ray &ray, float &intersectionDistance) const
{
    // Calculate intersection using barycentric coordinates. This gives a equation system
    // which we can solve using Cramer's rule.
    const glm::vec3  E1      = vertices[1] - vertices[0];
    const glm::vec3  E2      = vertices[2] - vertices[0];
    const glm::vec3  P       = glm::cross(ray.direction, E2);
    const glm::vec3  T       = ray.origin - vertices[0];
    const float      inv_den = 1.0f / glm::dot(E1, P);
    float            u       = inv_den * glm::dot(T, P);

    if ((u < 0.0f) || (u > 1.0f))
    {
        return false; // Didn't hit.
    }

    const glm::vec3  Q = glm::cross(T, E1);
    const float      v = inv_den * glm::dot(ray.direction, Q);

    if ((v < 0.0f) || (u + v > 1.0f))
    {
        return false; // Didn't hit.
    }

    intersectionDistance = inv_den * glm::dot(E2, Q);

    return intersectionDistance > std::numeric_limits<float>::min();
}
