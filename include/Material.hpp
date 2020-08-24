#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Material {
public:

    float refractiveIndex, reflectivity, transparency, emissivity, specularity,
          specularExponent;

    bool isEmissive() const
    {
        return emissivity > 0.0f;
    }

    bool isTransparent() const
    {
        return transparency > 0.0f;
    }

    bool isSpecular() const
    {
        return specularity > 0.0f;
    }

    bool isTotalReflective() const
    {
        return reflectivity > 0.0f;
    }

    bool isTotalTransparent() const
    {
        return fabs(transparency - 1.0f) < FLT_EPSILON;
    }

    virtual glm::vec3 getEmissionColor() const
    {
        return emissivity * getSurfaceColor();
    }

    virtual glm::vec3 getSurfaceColor() const = 0;

    virtual glm::vec3 calcDiffuseLighting(const glm::vec3& inDirection,
                                          const glm::vec3& outDirection,
                                          const glm::vec3& normal,
                                          const glm::vec3& incomingRadiance)
    const = 0;

    virtual glm::vec3 calcSpecularLighting(const glm::vec3& inDirection,
                                           const glm::vec3& outDirection,
                                           const glm::vec3& normal,
                                           const glm::vec3& incomingRadiance)
    const
    {
        return glm::vec3();
    }

    virtual ~Material()
    {}

protected:

    Material(float _emissivity   = 0.0f, float _reflectivity = 0.98f,
             float _transparency = 0.0f, float _refractiveIndex = 1.0f,
             float _specularity  = 0.0f, float _specularityExponent = 75.0f) :
        refractiveIndex(_refractiveIndex), reflectivity(_reflectivity), transparency(
            _transparency),
        emissivity(_emissivity), specularity(_specularity), specularExponent(
            _specularityExponent)
    {}
};

// A simple lambertian material using Blinn-Phong lighting model
class LambertianMaterial : public Material {
public:

    LambertianMaterial(glm::vec3 color,
                       float     emissivity       = 0.0f,
                       float     reflectivity     = 0.00f,
                       float     transparency     = 0.0f,
                       float     refractiveIndex  = 1.0f,
                       float     specularity      = 0.0f,
                       float     specularExponent = 75.0f) :
        Material(emissivity,
                 reflectivity,
                 transparency,
                 refractiveIndex,
                 specularity,
                 specularExponent), surfaceColor(color)
    {}

    glm::vec3 getSurfaceColor() const override
    {
        return surfaceColor;
    }

    glm::vec3 calcDiffuseLighting(const glm::vec3& inDirection,
                                  const glm::vec3& outDirection,
                                  const glm::vec3& normal,
                                  const glm::vec3& incomingIntensity) const
    override
    {
        float cosine = glm::max(0.0f, glm::dot(-inDirection, normal));

        return cosine * (incomingIntensity * surfaceColor);
    }

    virtual glm::vec3 calcSpecularLighting(const glm::vec3& inDirection,
                                           const glm::vec3& outDirection,
                                           const glm::vec3& normal,
                                           const glm::vec3& incomingRadiance) const
    override
    {
        glm::vec3 half = glm::normalize(outDirection - inDirection);
        float     sqr  = glm::pow(glm::dot(normal, half), specularExponent);

        return glm::max(0.0f, sqr) * incomingRadiance * specularity;
    }

private:

    glm::vec3 surfaceColor;
};
