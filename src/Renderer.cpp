#include "Renderer.h"

#include <algorithm>

#include "Math.hpp"

static const float  RAY_EPSILON = 0.001f;

Renderer::Renderer(const Scene &_scene, const unsigned int maxDepth):
    maxDepth(maxDepth), scene(_scene)
{
}

glm::vec3  Renderer::traceRay(const Ray &_ray, const unsigned int currentDepth) const
{
    if (currentDepth == maxDepth)
    {
        return glm::vec3(0);
    }

    // Epsilon
    Ray  ray(_ray.origin + RAY_EPSILON * _ray.direction, _ray.direction);

    // See if our current ray hits anything in the scene.
    float         intersectionDistance;
    unsigned int  intersectionTriangleIndex, intersectionRenderGroupIndex;
    const bool    intersectionFound = scene.rayCast(ray, intersectionRenderGroupIndex, intersectionTriangleIndex, intersectionDistance);

    // If the ray doesn't intersect, simply return (0, 0, 0).
    if (!intersectionFound)
    {
        return glm::vec3(0);
    }

    // Calculate intersection point.
    const glm::vec3  intersectionPoint = ray.origin + ray.direction * intersectionDistance;

    // Retrieve primitive information for the intersected object.
    auto       &intersectionRenderGroup = scene.renderGroups[intersectionRenderGroupIndex];
    const auto &intersectionTriangle    = intersectionRenderGroup.triangles[intersectionTriangleIndex];

    // Calculate hit normal.
    const glm::vec3  hitNormal = intersectionTriangle->getNormal(intersectionPoint);

    if (glm::dot(-ray.direction, hitNormal) < std::numeric_limits<float>::min())
    {
        return glm::vec3(0);
    }

    // Retrieve the intersected surface's material.
    const Material * const  hitMaterial = intersectionRenderGroup.material;

    // Emissive lighting.
    if (hitMaterial->isEmissive())
    {
        // float f = 1.0f;
        // if (DEPTH >= 1) {
        // f *= glm::dot(-ray.direction, hitNormal);
        // }
        auto  self = hitMaterial->calculateDiffuseLighting(-hitNormal, -ray.direction, hitNormal, hitMaterial->getEmissionColor());

        return hitMaterial->getEmissionColor() + self;
    }

    // Initialize color accumulator
    glm::vec3    colorAccumulator = glm::vec3(0);
    const float  rf               = 1.0f - hitMaterial->reflectivity;
    const float  tf               = 1.0f - hitMaterial->transparency;

    // Direct lighting
    if ((rf > std::numeric_limits<float>::min()) && (tf > std::numeric_limits<float>::min()))
    {
        for (Mesh *lightSource : scene.emissiveMesh)
        {
            // Create a shadow ray
            const glm::vec3  randomLightSurfacePosition = lightSource->getRandomPositionOnSurface();
            const glm::vec3  shadowRayDirection         = glm::normalize(randomLightSurfacePosition - intersectionPoint);

            if (glm::dot(shadowRayDirection, hitNormal) < std::numeric_limits<float>::min())
            {
                continue;
            }

            const Ray  shadowRay(intersectionPoint + hitNormal * RAY_EPSILON, shadowRayDirection);

            // Cast the shadow ray towards the light source
            unsigned int  shadowMeshIndex, shadowRayTriangleIndex;

            if (scene.rayCast(shadowRay, shadowMeshIndex, shadowRayTriangleIndex, intersectionDistance))
            {
                const auto &renderGroup = scene.renderGroups[shadowMeshIndex];

                if (&renderGroup == lightSource)
                {
                    // We hit the light. Add it's contribution to the color accumulator.
                    const Triangle  *lightTriangle = renderGroup.triangles[shadowRayTriangleIndex];
                    const glm::vec3  lightNormal   = lightTriangle->getNormal(shadowRay.origin + intersectionDistance * shadowRay.direction);
                    float            lightFactor   = glm::dot(-shadowRay.direction, lightNormal);

                    if (lightFactor < std::numeric_limits<float>::min())
                    {
                        continue;
                    }

                    // Direct diffuse lighting.
                    const glm::vec3  radiance = lightFactor * lightSource->material->getEmissionColor();
                    colorAccumulator += rf * tf * hitMaterial->calculateDiffuseLighting(-shadowRay.direction, -ray.direction, hitNormal, radiance);

                    // Specular lighting.
                    if (hitMaterial->isSpecular())
                    {
                        colorAccumulator += hitMaterial->calculateSpecularLighting(-shadowRay.direction, -ray.direction, hitNormal, radiance);
                    }
                }
            }
        }
    }

    colorAccumulator *= (1.0f / glm::max<float>(1.0f, (float)scene.emissiveMesh.size()));

    // Indirect lighting.
    if ((rf > std::numeric_limits<float>::min()) && (tf > std::numeric_limits<float>::min()))
    {
        // Shoot rays and integrate diffuse lighting based on BRDF to compute indirect lighting.
        const glm::vec3  reflectionDirection = Math::cosineWeightedHemisphereSampleDirection(hitNormal);
        const Ray        diffuseRay(intersectionPoint, reflectionDirection);
        const auto       incomingRadiance = traceRay(diffuseRay, currentDepth + 1);
        colorAccumulator += hitMaterial->calculateDiffuseLighting(-diffuseRay.direction, -ray.direction, hitNormal, incomingRadiance);
    }

    colorAccumulator *= rf * tf;

    // Mirror reflective lighting.
    if (hitMaterial->isReflective())
    {
        Ray  reflectedRay(intersectionPoint, glm::reflect(ray.direction, hitNormal));
        colorAccumulator += hitMaterial->reflectivity * traceRay(reflectedRay, currentDepth + 1);
    }

    // Refracted lighting.
    if (hitMaterial->isTransparent())
    {
        // Fetch refractive data.
        const float  n1                     = 1.0f;
        const float  n2                     = hitMaterial->refractiveIndex;
        const float  schlickConstantOutside = Math::calculateSchlicksApproximation(ray.direction, hitNormal, n1, n2);

        // Refract ray.
        glm::vec3  offset = hitNormal * RAY_EPSILON;
        Ray        refractedRay(intersectionPoint - offset, glm::refract(ray.direction, hitNormal, n1 / n2));

        if (scene.renderGroupRayCast(refractedRay, intersectionRenderGroupIndex, intersectionTriangleIndex, intersectionDistance))
        {
            const auto      &refractedRayHitTriangle    = intersectionRenderGroup.triangles[intersectionTriangleIndex];
            const glm::vec3  refractedIntersectionPoint = refractedRay.origin + refractedRay.direction * intersectionDistance;
            const glm::vec3  refractedHitNormal         = refractedRayHitTriangle->getNormal(refractedIntersectionPoint);
            float            schlickConstantInside      = Math::calculateSchlicksApproximation(refractedRay.direction, -refractedHitNormal, n2, n1);
            Ray              refractedRayOut(refractedIntersectionPoint + 0.01f * refractedHitNormal, glm::refract(refractedRay.direction, -refractedHitNormal,
                                                                                                                   n2 / n1));
            const float  f1               = (1.0f - schlickConstantOutside) * (hitMaterial->transparency);
            const float  f2               = (1.0f - schlickConstantInside);
            const auto   incomingRadiance = f2 * traceRay(refractedRayOut, currentDepth + 1);
            colorAccumulator += f1 * hitMaterial->calculateDiffuseLighting(refractedRay.direction, -ray.direction, hitNormal, incomingRadiance);
        }
        else
        {
            colorAccumulator += (1.0f - schlickConstantOutside) * (hitMaterial->transparency) * traceRay(refractedRay, currentDepth + 1);
        }

        Ray          specularRay(intersectionPoint, glm::reflect(ray.direction, hitNormal));
        const float  sf = schlickConstantOutside * hitMaterial->specularity;
        colorAccumulator += sf
                            * hitMaterial->calculateSpecularLighting(-specularRay.direction, -ray.direction, hitNormal,
                                                                     traceRay(specularRay, currentDepth + 1));
    }

    // Return result.
    return colorAccumulator;
}
