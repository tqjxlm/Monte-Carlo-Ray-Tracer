#include "Renderer.h"

#include <algorithm>

#include "Math.hpp"

static const float RAY_EPSILON = 0.001f;

Renderer::Renderer(const Scene& _scene, const unsigned int maxDepth) : maxDepth(maxDepth), scene(_scene)
{}

glm::vec3 Renderer::traceRay(const Ray& _ray, const unsigned int currentDepth) const
{
    if (currentDepth == maxDepth)
    {
        return glm::vec3(0);
    }

    // Epsilon: avoid self intersection
    Ray ray(_ray.origin + RAY_EPSILON * _ray.direction, _ray.direction);

    // See if our current ray hits anything in the scene
    float intersectedDistance;
    unsigned int intersectedTriangleID, intersectedGroupID;
    const bool   intersectionFound = scene.rayCast(ray, intersectedGroupID, intersectedTriangleID, intersectedDistance);

    // If the ray doesn't intersect, simply return (0, 0, 0)
    if (!intersectionFound)
    {
        return glm::vec3(0);
    }

    // Calculate intersection point.
    const glm::vec3 intersectedPoint = ray.origin + ray.direction * intersectedDistance;

    // Retrieve primitive information for the intersected object
    const auto& intersectedGroup    = scene.getRenderGroup(intersectedGroupID);
    const auto  intersectedTriangle = intersectedGroup.triangles[intersectedTriangleID];
    const glm::vec3 hitNormal       = intersectedTriangle->getNormal(intersectedPoint);

    // Back face culling
    if (glm::dot(-ray.direction, hitNormal) < std::numeric_limits<float>::min())
    {
        return glm::vec3(0);
    }

    // Retrieve the intersected surface's material
    const Material * const hitMaterial = intersectedGroup.material;

    // Emissive lighting (ending point for any tracing path)
    if (hitMaterial->isEmissive())
    {
        if (currentDepth == 0)
        {
            // Since we use explicit light sampling later, we should add an
            // additional emissive light in this case
            return hitMaterial->getEmissionColor() +
                   glm::dot(-ray.direction, hitNormal) * hitMaterial->getEmissionColor();
        }
        else
        {
            return glm::dot(-ray.direction, hitNormal) * hitMaterial->getEmissionColor();
        }
    }

    // Initialize color accumulator
    glm::vec3 colorAccumulator = glm::vec3(0);
    bool shouldDiffuse         = !hitMaterial->isTotalReflective() && !hitMaterial->isTotalTransparent();

    // Explicit light sampling
    // https://computergraphics.stackexchange.com/questions/5152/progressive-path-tracing-with-explicit-light-sampling
    if (shouldDiffuse)
    {
        for (Mesh* lightSource : scene.getEmissiveMeshes())
        {
            // Create a shadow ray
            const glm::vec3 randomLightSurfacePosition = lightSource->getRandomPositionOnSurface();
            const glm::vec3 shadowRayDirection         = glm::normalize(randomLightSurfacePosition - intersectedPoint);

            if (glm::dot(shadowRayDirection, hitNormal) < std::numeric_limits<float>::min())
            {
                continue;
            }

            const Ray shadowRay(intersectedPoint + hitNormal * RAY_EPSILON,
                                shadowRayDirection);

            // Cast the shadow ray towards the light source
            unsigned int shadowMeshIndex, shadowRayTriangleIndex;

            if (scene.rayCast(shadowRay, shadowMeshIndex, shadowRayTriangleIndex, intersectedDistance))
            {
                const auto& renderGroup = scene.getRenderGroup(shadowMeshIndex);

                if (&renderGroup == lightSource)
                {
                    // We hit the light. Add it's contribution to the color
                    // accumulator.
                    const Triangle* lightTriangle = renderGroup.triangles[shadowRayTriangleIndex];
                    const glm::vec3 lightNormal   = lightTriangle->getNormal(
                        shadowRay.origin + intersectedDistance * shadowRay.direction);
                    float lightFactor = glm::dot(-shadowRay.direction, lightNormal);

                    if (lightFactor < std::numeric_limits<float>::min())
                    {
                        continue;
                    }

                    // Direct diffuse lighting.
                    const glm::vec3 radiance = lightFactor * lightSource->material->getEmissionColor();
                    colorAccumulator += hitMaterial->calcDiffuseLighting(
                        -shadowRay.direction, -ray.direction, hitNormal, radiance);

                    // Specular lighting.
                    if (hitMaterial->isSpecular())
                    {
                        colorAccumulator += hitMaterial->calcSpecularLighting(
                            -shadowRay.direction,
                            -ray.direction,
                            hitNormal,
                            radiance);
                    }
                }
            }
        }

        // Avoid over-lighting.
        colorAccumulator /= glm::max<float>(1.0f, (float)scene.getEmissiveMeshes().size());
    }

    // Indirect lighting (diffuse light)
    if (shouldDiffuse)
    {
        // Shoot rays and integrate diffuse lighting based on BRDF to compute
        // indirect lighting.
        const glm::vec3 reflectionDirection = Math::sampleHemisphereWeighted(hitNormal);
        const Ray  diffuseRay(intersectedPoint + hitNormal * RAY_EPSILON, reflectionDirection);
        const auto incomingRadiance = traceRay(diffuseRay, currentDepth + 1);
        colorAccumulator += hitMaterial->calcDiffuseLighting(
            -diffuseRay.direction, -ray.direction,
            hitNormal, incomingRadiance);

        // Color blending when material is reflective or transparent
        colorAccumulator *= (1.0f - hitMaterial->reflectivity) * (1.0f - hitMaterial->transparency);
    }

    // Reflected light only
    if (hitMaterial->isTotalReflective())
    {
        const glm::vec3 direction = glm::reflect(ray.direction, hitNormal);
        Ray reflectedRay(intersectedPoint + hitNormal * RAY_EPSILON, direction);
        colorAccumulator += hitMaterial->reflectivity * traceRay(reflectedRay, currentDepth + 1);
    }

    // Refracted light + reflected light
    else if (hitMaterial->isTransparent())
    {
        // Schick approximation is used to determine how much light is refracted or reflected
        const float n1                     = 1.0f;
        const float n2                     = hitMaterial->refractiveIndex;
        const float schlickConstantOutside = Math::schlicksApprox(ray.direction, hitNormal, n1, n2);
        Ray refractedRay(intersectedPoint - hitNormal * RAY_EPSILON, glm::refract(ray.direction, hitNormal, n1 / n2));
        const float fRefracted = (1.0f - schlickConstantOutside) * hitMaterial->transparency;

        if (scene.renderGroupRayCast(refractedRay, intersectedGroupID,
                                     intersectedTriangleID, intersectedDistance))
        {
            // Self-intersected, cast ray from the exit point to the outer world and do refrection twice
            const auto& refractedRayHitTriangle = intersectedGroup.triangles[intersectedTriangleID];
            const glm::vec3 refractedPoint      = refractedRay.origin + refractedRay.direction * intersectedDistance;
            const glm::vec3 refractedHitNormal  = refractedRayHitTriangle->getNormal(refractedPoint);
            float schlickConstantInside         = Math::schlicksApprox(refractedRay.direction,
                                                                       -refractedHitNormal,
                                                                       n2,
                                                                       n1);

            const glm::vec3 direction = glm::refract(refractedRay.direction, -refractedHitNormal, n2 / n1);
            Ray refractedRayOut(refractedPoint + RAY_EPSILON * refractedHitNormal, direction);
            const float fRefractedIn = (1.0f - schlickConstantInside);

            // Don't increase depth for refracted rays
            auto incomingRadiance = fRefractedIn * traceRay(refractedRayOut, currentDepth);
            colorAccumulator += fRefracted * hitMaterial->calcDiffuseLighting(
                refractedRay.direction, -ray.direction, hitNormal, incomingRadiance);
        }
        else
        {
            // Not self-intersected, refract only once
            colorAccumulator += fRefracted * traceRay(refractedRay, currentDepth + 1);
        }

        // The remaining ray is reflected
        auto outDirection = glm::reflect(ray.direction, hitNormal);
        Ray  specularRay(intersectedPoint + hitNormal * RAY_EPSILON, outDirection);
        const float fReflected = schlickConstantOutside * hitMaterial->transparency;
        colorAccumulator += fReflected * traceRay(specularRay, currentDepth + 1);
    }

    return colorAccumulator;
}
