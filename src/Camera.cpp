#include "Camera.h"

#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>

#include "Ray.hpp"
#include "Math.hpp"

static const double LOG_INTERVAL = 0.3;
static const float  GAMMA        = 0.6f;

inline void toHumanTime(long long time, long long& hours, long long& mins, long long& secs)
{
    secs  = time % 60;
    mins  = (time / 60) % 60;
    hours = ((time / 60) / 60);
}

Camera::Camera(const unsigned int _width, const unsigned int _height) : width(_width), height(_height)
{
    pixels.assign(width, std::vector<Pixel>(height));
    discretizedPixels.assign(width, std::vector<glm::u8vec3>(height));

    totalPixels   = width * height;
    currentPixels = 0;
}

void Camera::render(const Scene& scene,
                    Renderer   & renderer,
                    unsigned int samplePerPixel,
                    glm::vec3    eye,
                    glm::vec3    direction,
                    glm::vec3    up)
{
    // Calculate the retina plane (which receives rays)
    direction = glm::normalize(direction);

    glm::vec3 center = eye + direction * 2.0f;
    glm::vec3 right  = glm::normalize(glm::cross(direction, up));
    up = glm::normalize(glm::cross(right, direction));

    glm::vec3  c1        = center + right - up;
    glm::vec3  c2        = center - right - up;
    glm::vec3  c3        = center - right + up;
    glm::vec3  c4        = center + right + up;
    const auto startTime = std::chrono::high_resolution_clock::now();

    // Initialize the random engine
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> rand(0, 1.0f - std::numeric_limits<float>::min());

    // Precompute inverse widths and heights
    const float invWidth  = 1.0f / static_cast<float>(width);
    const float invHeight = 1.0f / static_cast<float>(height);
    const float invSample = 1.0f / static_cast<float>(samplePerPixel);

    // Calculate step lengths
    const float sqrtSamplePerPixel    = sqrtf(static_cast<float>(samplePerPixel));
    const float invSqrtSamplePerPixel = 1.0f / sqrtSamplePerPixel;
    const float columnStep            = invWidth * invSqrtSamplePerPixel;
    const float rowStep               = invHeight * invSqrtSamplePerPixel;

    // Camera plane normal
    const glm::vec3 viewPlaneNormal = -glm::normalize(glm::cross(c1 - c2, c1 - c4));

    // Shoot multiple rays through every pixel
    for (int y = 0; y < static_cast<int>(width); ++y)
    {
#pragma omp parallel for

        for (int z = 0; z < static_cast<int>(height); ++z)
        {
            // Shoot a bunch of rays through the pixel (y, z), and accumulate colors
            Ray ray;
            glm::vec3 colorAccumulator(0);

            for (float c = 0; c < invWidth - columnStep + std::numeric_limits<float>::min();
                 c += columnStep)
            {
                for (float r = 0; r < invHeight - rowStep + std::numeric_limits<float>::min();
                     r += rowStep)
                {
                    // Calculate camera plane ray position using stratified sampling
                    const float ylerp = y * invWidth + c + rand(gen) * columnStep;
                    const float zlerp = z * invHeight + r + rand(gen) * rowStep;
                    const float nx    = Math::bilinearInterpolation(ylerp, zlerp, c1.x, c2.x, c3.x, c4.x);
                    const float ny    = Math::bilinearInterpolation(ylerp, zlerp, c1.y, c2.y, c3.y, c4.y);
                    const float nz    = Math::bilinearInterpolation(ylerp, zlerp, c1.z, c2.z, c3.z, c4.z);

                    // Create ray
                    ray.origin    = glm::vec3(nx, ny, nz);
                    ray.direction = glm::normalize(ray.origin - eye);
                    const float rayFactor =
                        std::max(0.0f, glm::dot(-ray.direction, viewPlaneNormal));

                    // Shoot ray
                    colorAccumulator += rayFactor * renderer.getPixelColor(ray);
                }
            }

            // Set pixel color dependent on the traced ray
            pixels[y][z].color = invSample * colorAccumulator;

            logProgress();
        }
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto took = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    long long  hours, mins, secs;
    toHumanTime(took / 1000, hours, mins, secs);
    printf("\nRendering finished. Total time:  %02lld: %02lld: %02lld.\n", hours, mins, secs);

    // Create the final discretized image. Should always be done immediately after the rendering step
    createImage();
}

bool Camera::writeImageTGA(const std::string path) const
{
    // Initialize
    std::ofstream o(path.c_str(), std::ios::out | std::ios::binary);

    // Write header
    const std::string header = "002000000000";

    for (unsigned int i = 0; i < header.length(); ++i)
    {
        o.put(header[i] - '0');
    }

    o.put(width & 0x00FF);
    o.put((width & 0xFF00) >> 8);
    o.put(height & 0x00FF);
    o.put((height & 0xFF00) >> 8);
    o.put(32); // 32 bit bitmap
    o.put(0);

    // Write data
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            auto& cp = discretizedPixels[x][y];
            o.put(cp.b);
            o.put(cp.g);
            o.put(cp.r);
            o.put((char)0xFF); // 255
        }
    }

    o.flush();
    o.close();

    return true;
}

void Camera::createImage()
{
    // Find max color intensity
    float maxIntensity = 0;

    for (size_t i = 0; i < width; ++i)
    {
        for (size_t j = 0; j < height; ++j)
        {
            const auto& c = pixels[i][j].color;
            maxIntensity = std::max<float>(c.r, maxIntensity);
            maxIntensity = std::max<float>(c.g, maxIntensity);
            maxIntensity = std::max<float>(c.b, maxIntensity);
        }
    }

    // GAMMA correction
    for (size_t i = 0; i < width; ++i)
    {
        for (size_t j = 0; j < height; ++j)
        {
            pixels[i][j].color = glm::pow(pixels[i][j].color, glm::vec3(GAMMA));
        }
    }

    maxIntensity = glm::pow(maxIntensity, GAMMA);

    // Discretize pixels using the max intensity
    // Every discretized value must be between 0 and 255
    glm::u8 discretizedMaxIntensity{};
    const float f = 254.99f / maxIntensity;

    for (size_t i = 0; i < width; ++i)
    {
        for (size_t j = 0; j < height; ++j)
        {
            const auto c = f * pixels[i][j].color;
            discretizedPixels[i][j].r = (glm::u8)round(c.r);
            discretizedPixels[i][j].g = (glm::u8)round(c.g);
            discretizedPixels[i][j].b = (glm::u8)round(c.b);
            discretizedMaxIntensity   = glm::max(discretizedMaxIntensity, discretizedPixels[i][j].r);
            discretizedMaxIntensity   = glm::max(discretizedMaxIntensity, discretizedPixels[i][j].g);
            discretizedMaxIntensity   = glm::max(discretizedMaxIntensity, discretizedPixels[i][j].b);
        }
    }

    std::cout << "Image created from render results. Max intensity: " << maxIntensity << std::endl;
}

inline void Camera::logProgress()
{
    using namespace std::chrono;
    static const auto startTime = high_resolution_clock::now();
    static auto lastLog         = high_resolution_clock::now();

    currentPixels++;
    const auto now = high_resolution_clock::now();

    // Estimate time left
    auto elapsedTime               = duration_cast<milliseconds>(now - startTime).count();
    const double percentageDone    = 100 * (currentPixels / (double)totalPixels);
    const double percentageLeft    = (100 - percentageDone);
    long long    estimatedTimeLeft = (long long)llround((elapsedTime / percentageDone) * percentageLeft / 1000);

    // Log once a while
    const double timeSinceLastLog = (double)duration_cast<milliseconds>(now - lastLog).count() / 1000;

    if (timeSinceLastLog > LOG_INTERVAL)
    {
        lastLog = now;
        long long hours, mins, secs;
        toHumanTime(estimatedTimeLeft, hours, mins, secs);
        printf("\rProgress %.1lf%%. Estimated remaining %02lld: %02lld: %02lld.", percentageDone, hours, mins, secs);
    }
}
