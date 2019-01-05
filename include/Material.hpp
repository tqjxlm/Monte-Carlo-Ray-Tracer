#pragma once

#include <glm/glm.hpp>

#include <glm/gtc/constants.hpp>

class Material
{
public:
    float  refractiveIndex, reflectivity, transparency, emissivity, specularity, specularExponent;

    bool  isEmissive() const
    {
        return emissivity > std::numeric_limits<float>::min();
    }

    bool  isTransparent() const
    {
        return transparency > std::numeric_limits<float>::min();
    }

    bool  isSpecular() const
    {
        return specularity > std::numeric_limits<float>::min();
    }

    bool  isReflective() const
    {
        return reflectivity > std::numeric_limits<float>::min();
    }

    virtual glm::vec3  getEmissionColor() const
    {
        return emissivity * getSurfaceColor();
    }

    virtual glm::vec3  getSurfaceColor() const = 0;

    virtual glm::vec3  calculateDiffuseLighting(const glm::vec3 &inDirection, const glm::vec3 &outDirection,
                                                const glm::vec3 &normal, const glm::vec3 &incomingRadiance) const = 0;

    virtual glm::vec3  calculateSpecularLighting(const glm::vec3 &inDirection, const glm::vec3 &outDirection,
                                                 const glm::vec3 &normal, const glm::vec3 &incomingRadiance) const
    {
        // const glm::vec3 lightReflection = glm::reflect(inDirection, normal);
        const glm::vec3  half = glm::normalize(outDirection - inDirection);
        float            sqr  = glm::pow<float>(glm::dot(normal, half), specularExponent);

        return glm::max(0.0f, sqr) * incomingRadiance;
    }

protected:
    Material(float _emissivity   = 0.0f, float _reflectivity = 0.98f,
             float _transparency = 0.0f, float _refractiveIndex = 1.0f,
             float _specularity  = 0.0f, float _specularityExponent = 75.0f):
        refractiveIndex(_refractiveIndex), transparency(_transparency),
        emissivity(_emissivity), reflectivity(_reflectivity), specularity(_specularity), specularExponent(_specularityExponent)
    {
    }
};

class LambertianMaterial: public Material
{
public:
    LambertianMaterial(glm::vec3 color, float emissivity = 0.0f, float reflectivity = 0.00f,
                       float transparency                = 0.0f, float refractiveIndex = 1.0f, float specularity = 0.0f, float specularExponent = 75.0f):
        surfaceColor(color), Material(emissivity, reflectivity, transparency, refractiveIndex, specularity, specularExponent)
    {
    }

    glm::vec3  getSurfaceColor() const override
    {
        return surfaceColor;
    }

    glm::vec3  calculateDiffuseLighting(const glm::vec3 &inDirection, const glm::vec3 &outDirection,
                                        const glm::vec3 &normal, const glm::vec3 &incomingIntensity) const override
    {
        return glm::max(0.0f, glm::dot(-inDirection, normal)) * (incomingIntensity * surfaceColor);
    }

private:
    glm::vec3  surfaceColor;
};
