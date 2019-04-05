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

    // Epsilon: avoid self intersection
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
    const auto      &intersectionRenderGroup = scene.getRenderGroup(intersectionRenderGroupIndex);
    const auto       intersectionTriangle    = intersectionRenderGroup.triangles[intersectionTriangleIndex];
    const glm::vec3  hitNormal               = intersectionTriangle->getNormal(intersectionPoint);

    // Back face culling
    if (glm::dot(-ray.direction, hitNormal) < std::numeric_limits<float>::min())
    {
        return glm::vec3(0);
    }

    // Retrieve the intersected surface's material.
    const Material * const  hitMaterial = intersectionRenderGroup.material;

    // Emissive lighting (ending point for any tracing path).
    if (hitMaterial->isEmissive())
    {
        // TODO: Not sure why light sources are not bright enough,
        // an additional light should be added to compensate
        if (currentDepth == 0)
        {
            return hitMaterial->getEmissionColor();
        }
        else
        {
            return glm::dot(-ray.direction, hitNormal) * hitMaterial->getEmissionColor();
        }
    }

    // Initialize color accumulator
    glm::vec3    colorAccumulator = glm::vec3(0);
    const float  rf               = 1.0f - hitMaterial->reflectivity;
    const float  tf               = 1.0f - hitMaterial->transparency;
    bool         shouldDiffuse    = (rf > std::numeric_limits<float>::min()) && (tf > std::numeric_limits<float>::min());

    // Direct lighting
    if (shouldDiffuse)
    {
        for (Mesh *lightSource : scene.getEmissiveMeshes())
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
                const auto &renderGroup = scene.getRenderGroup(shadowMeshIndex);

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
                    colorAccumulator += hitMaterial->calculateDiffuseLighting(-shadowRay.direction, -ray.direction, hitNormal, radiance);

                    // Specular lighting.
                    if (hitMaterial->isSpecular())
                    {
                        colorAccumulator += hitMaterial->calculateSpecularLighting(-shadowRay.direction, -ray.direction, hitNormal, radiance);
                    }
                }
            }
        }

        // Avoid over-lighting.
        colorAccumulator /= glm::max<float>(1.0f, (float)scene.getEmissiveMeshes().size());
    }

    // Indirect lighting.
    if (shouldDiffuse)
    {
        // Shoot rays and integrate diffuse lighting based on BRDF to compute indirect lighting.
        const glm::vec3  reflectionDirection = Math::cosineWeightedHemisphereSampleDirection(hitNormal);
        const Ray        diffuseRay(intersectionPoint + hitNormal * RAY_EPSILON, reflectionDirection);
        const auto       incomingRadiance = traceRay(diffuseRay, currentDepth + 1);
        colorAccumulator += hitMaterial->calculateDiffuseLighting(-diffuseRay.direction, -ray.direction, hitNormal, incomingRadiance);
    }

    // Color blending when material is reflective or transparent
    colorAccumulator *= rf * tf;

    // Mirror reflective lighting.
    if (hitMaterial->isReflective())
    {
        const float  n1            = 1.0f;
        const float  n2            = rf;
        const float  reflectFactor = Math::calculateSchlicksApproximation(ray.direction, hitNormal, n1, n2);
        Ray          reflectedRay(intersectionPoint + hitNormal * RAY_EPSILON, glm::reflect(ray.direction, hitNormal));

        colorAccumulator += reflectFactor * traceRay(reflectedRay, currentDepth + 1);
    }
    // Refracted lighting. (Reflectiveness is a special case of refractiveness, so they won't happen in the same time)
    else if (hitMaterial->isTransparent())
    {
        const float  n1                     = 1.0f;
        const float  n2                     = hitMaterial->refractiveIndex;
        const float  schlickConstantOutside = Math::calculateSchlicksApproximation(ray.direction, hitNormal, n1, n2);
        Ray          refractedRay(intersectionPoint - hitNormal * RAY_EPSILON, glm::refract(ray.direction, hitNormal, n1 / n2));

        if (scene.renderGroupRayCast(refractedRay, intersectionRenderGroupIndex, intersectionTriangleIndex, intersectionDistance))
        {
            // Self-intersected, cast ray from the other point to the outer world and do refrection twice
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
            // Not self-intersected, refract only once
            colorAccumulator += (1.0f - schlickConstantOutside) * (hitMaterial->transparency) * traceRay(refractedRay, currentDepth + 1);
        }

        // The remaining ray is reflected
        Ray          specularRay(intersectionPoint + hitNormal * RAY_EPSILON, glm::reflect(ray.direction, hitNormal));
        const float  sf = schlickConstantOutside * hitMaterial->specularity;
        colorAccumulator += sf
                            * hitMaterial->calculateSpecularLighting(-specularRay.direction, -ray.direction, hitNormal,
                                                                     traceRay(specularRay, currentDepth + 1));
    }

    // Return result.
    return colorAccumulator;
}
