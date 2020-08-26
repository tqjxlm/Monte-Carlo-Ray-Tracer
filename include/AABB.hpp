#ifndef AABBOX_H
#define AABBOX_H

#include <algorithm>

#include "Ray.hpp"
#include "Math.hpp"

// Axis-aligned bounding box
class AABB {
public:

    AABB(glm::vec3 bl_ = glm::vec3(), glm::vec3 tr_ = glm::vec3())
        : bl(bl_), tr(tr_)
    {
    }

    // Expand to fit box
    void expand(const AABB& box)
    {
        if (box.bl.x < bl.x) bl.x = box.bl.x;

        if (box.bl.y < bl.y) bl.y = box.bl.y;

        if (box.bl.z < bl.z) bl.z = box.bl.z;

        if (box.tr.x > tr.x) tr.x = box.tr.x;

        if (box.tr.y > tr.y) tr.y = box.tr.y;

        if (box.tr.z > tr.z) tr.z = box.tr.z;
    }

    // Expand to fit point
    void expand(const glm::vec3& vec)
    {
        if (vec.x < bl.x) bl.x = vec.x;

        if (vec.y < bl.y) bl.y = vec.y;

        if (vec.z < bl.z) bl.z = vec.z;
    }

    // Returns longest axis: 0, 1, 2 for x, y, z respectively
    int get_longest_axis()
    {
        glm::vec3 diff = tr - bl;

        if ((diff.x > diff.y) && (diff.x > diff.z)) return 0;

        if ((diff.y > diff.x) && (diff.y > diff.z)) return 1;

        return 2;
    }

    // Check if ray intersects with box. Returns true/false and stores distance in t
    bool intersection(const Ray& r, float& t)
    {
        // TODO: The release build of these functions will cause issues, not sure why
        //glm::vec3 t_bl = (bl - r.origin) * r.direction_inv;
        //glm::vec3 t_tr = (tr - r.origin) * r.direction_inv;
        //glm::vec3 tMax = glm::max(t_bl, t_tr);
        //glm::vec3 tMin = glm::min(t_bl, t_tr);

        glm::vec3 tMin = (bl - r.origin) * r.direction_inv;
        glm::vec3 tMax = (tr - r.origin) * r.direction_inv;

        Math::sortByComponent(tMax, tMin);

        float minOfMax = std::min(tMax.x, std::min(tMax.y, tMax.z));
        if (minOfMax < 0)
        {
            t = minOfMax;
            return false;
        }

        float maxOfMin = std::max(tMin.x, std::max(tMin.y, tMin.z));
        t = maxOfMin > 0 ? maxOfMin : minOfMax;

        return minOfMax >= maxOfMin;
    }

private:

    glm::vec3 bl; // Bottom left (min)
    glm::vec3 tr; // Top right   (max)
};

#endif // AABBOX_H
