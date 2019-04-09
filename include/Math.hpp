#pragma once

#include <random>
#include <cassert>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <string>
#include <ctime>
#include <chrono>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Math
{
// Returns a vector with indices sorted based on the values in the vector /values/.
static inline std::vector<int>  getSortedIndices(const std::vector<float> &values)
{
    std::vector<int>  indices(values.size());

    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&values](size_t i1, size_t i2)
        {
            return values[i1] < values[i2];
        });

    return indices;
}

static inline glm::vec3  calcNormal(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3)
{
    return glm::normalize(glm::cross(v3 - v2, v1 - v2));
}

// Returns an bilinearly interpolated value between 4 corner values.
static inline float  bilinearInterpolation(const float dy, const float dz,
                                           const float x1, const float x2, const float x3, const float x4)
{
    const float  idy = 1 - dy;
    const float  idz = 1 - dz;
    float        a1  = idy * dz; // lower right area.
    float        a2  = dy * dz; // lower left area.
    float        a3  = dy * idz; // upper left area.
    float        a4  = idy * idz; // upper right area.

    return a3 * x1 + a4 * x2 + a1 * x3 + a2 * x4;
}

// A normal distribution generator that generates numbers between two given numbers.
// There will be a small peak at min and max (since normal distributions are not really
// supposed to be limited to a range).
class NormalDistributionGenerator
{
public:
    NormalDistributionGenerator(float min = 0.0f, float max = glm::half_pi<float>()):
        min(min), max(max), distribution(0.5f * (min + max), (1.0f / 6.0f) * (max - min))
    {
    }

    float  getRandomFloat()
    {
        float  v = distribution(generator);

        if (v < min - std::numeric_limits<float>::min()) { return min; }

        if (v > max + std::numeric_limits<float>::min()) { return max; }

        return v;
    }

private:
    std::default_random_engine       generator;
    float                            min, max;
    std::normal_distribution<float>  distribution;
};

// Returns a vector that is non-parallell to a given vector.
static inline glm::vec3  nonParallellVector(const glm::vec3 &v)
{
    if (abs(v.x) < std::numeric_limits<float>::min())
    {
        return glm::vec3(1, 0, 0);
    }
    else if (abs(v.y) < std::numeric_limits<float>::min())
    {
        return glm::vec3(0, 1, 0);
    }
    else if (abs(v.z) < std::numeric_limits<float>::min())
    {
        return glm::vec3(0, 0, 1);
    }
    else
    {
        // None of v-parameters are zero.
        return glm::vec3(-v.y, v.z, -v.x);
    }
}

// Returns a random direction given a normal.
// Uses cosine-weighted hemisphere sampling.
static inline glm::vec3  cosineWeightedHemisphereSampleDirection(const glm::vec3 &n)
{
    // Samples cosine weighted positions.
    float      r1    = rand() / static_cast<float>(RAND_MAX);
    float      r2    = rand() / static_cast<float>(RAND_MAX);
    float      theta = acos(sqrt(1.0f - r1));
    float      phi   = 2.0f * glm::pi<float>() * r2;
    float      xs    = sinf(theta) * cosf(phi);
    float      ys    = cosf(theta);
    float      zs    = sinf(theta) * sinf(phi);
    glm::vec3  y(n.x, n.y, n.z);
    glm::vec3  h = y;

    if ((abs(h.x) <= abs(h.y)) && (abs(h.x) <= abs(h.z)))
    {
        h.x = 1.0;
    }
    else if ((abs(h.y) <= abs(h.x)) && (abs(h.y) <= abs(h.z)))
    {
        h.y = 1.0;
    }
    else
    {
        h.z = 1.0;
    }

    glm::vec3  x = glm::normalize(glm::cross(h, y));
    glm::vec3  z = glm::normalize(glm::cross(x, y));

    return glm::normalize(xs * x + ys * y + zs * z);
}

// Returns a random direction given a normal.
// Uses uniform randomization.
static inline glm::vec3  randomHemishpereSampleDirection(const glm::vec3 &n)
{
    // Samples uniform angles.
    float      incl               = (rand() / static_cast<float>(RAND_MAX)) * glm::half_pi<float>();
    float      azim               = (rand() / static_cast<float>(RAND_MAX)) * glm::two_pi<float>();
    glm::vec3  nonParallellVector = Math::nonParallellVector(n);

    assert(glm::length(glm::cross(nonParallellVector, n)) > std::numeric_limits<float>::min());
    glm::vec3  rotationVector = glm::cross(nonParallellVector, n);
    glm::vec3  inclVector     = rotate(n, incl, rotationVector);

    return glm::normalize(rotate(inclVector, azim, n));
}

static inline float  calculateSchlicksApproximation(const glm::vec3 &incomingDirection,
                                                    const glm::vec3 &normal, float n1 = 1.0f, float n2 = 1.0f)
{
    float  R0    = glm::pow((n1 - n2) / (n1 + n2), 2.0f);
    float  alpha = glm::dot(normal, -incomingDirection);

    return R0 + (1 - R0) * glm::pow((1 - alpha), 5.0f);
}

// Returns a string that represents the current date and time.
static inline std::string  currentDateTime()
{
    auto       now = time(0);
    struct tm  tstruct;

    localtime_s(&tstruct, &now);
    std::string  date = std::to_string(tstruct.tm_year) + "-"
                        + std::to_string(tstruct.tm_mon) + "-" + std::to_string(tstruct.tm_mday);
    std::string  time = std::to_string(tstruct.tm_hour) + "-"
                        + std::to_string(tstruct.tm_min) + "-" + std::to_string(tstruct.tm_sec);

    return date + "___" + time;
}
}
