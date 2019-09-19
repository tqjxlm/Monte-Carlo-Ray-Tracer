#pragma once

#include <glm/glm.hpp>

#include "Ray.hpp"
#include "AABB.hpp"
#include "Math.hpp"

class Triangle {
public:

    Triangle(glm::vec3 _v1,
             glm::vec3 _v2,
             glm::vec3 _v3,
             glm::vec3 _n1,
             glm::vec3 _n2,
             glm::vec3 _n3,
             int       index);

    Triangle(glm::vec3 _v1, glm::vec3 _v2, glm::vec3 _v3, glm::vec3 _norm, int index)
        : vertices{_v1, _v2, _v3}, edges{_v2 - _v1, _v3 - _v1}, faceNorm(_norm), meshIndex(index)
    {
        flat = true;
    }

    Triangle(glm::vec3 _v1, glm::vec3 _v2, glm::vec3 _v3, int index)
        : vertices{_v1, _v2, _v3}, edges{_v2 - _v1, _v3 - _v1}, meshIndex(index)
    {
        flat     = true;
        faceNorm = Math::calcNormal(_v1, _v2, _v3);
    }

    glm::vec3 getNormal(const glm::vec3& position) const
    {
        if (flat)
        {
            return faceNorm;
        }

        auto w  = position - vertices[0];
        auto wv = glm::dot(w, v);
        auto wu = glm::dot(w, u);
        auto s  = (uv * wv - vv * wu) / fractor;
        auto t  = (uv * wu - uu * wv) / fractor;

        return glm::normalize(normals[1] * s + normals[2] * t + normals[0] * (1 - s - t));
    }

    glm::vec3 getCenter() const
    {
        return (vertices[0] + vertices[1] + vertices[2]) / 3.0f;
    }

    glm::vec3 getRandomPositionOnSurface() const;

    AABB      getBoundingBox()
    {
        glm::vec3 bl = glm::vec3(
            std::min(std::min(vertices[0].x, vertices[1].x), vertices[2].x),
            std::min(std::min(vertices[0].y, vertices[1].y), vertices[2].y),
            std::min(std::min(vertices[0].z, vertices[1].z), vertices[2].z));
        glm::vec3 tr = glm::vec3(
            std::max(std::max(vertices[0].x, vertices[1].x), vertices[2].x),
            std::max(std::max(vertices[0].y, vertices[1].y), vertices[2].y),
            std::max(std::max(vertices[0].z, vertices[1].z), vertices[2].z));

        return AABB(bl, tr);
    }

    bool rayIntersection(const Ray& ray,
                         float    & intersectionDistance) const;

public:

    bool enabled = true;
    bool convex  = true;
    glm::vec3 vertices[3];
    glm::vec3 normals[3];
    glm::vec3 edges[2];
    glm::vec3 faceNorm;
    int meshIndex;

private:

    glm::vec3 u;
    glm::vec3 v;
    float uu;
    float vv;
    float uv;
    float fractor;
    bool flat;
};
