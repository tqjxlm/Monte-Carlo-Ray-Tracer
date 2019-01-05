#ifndef AABBOX_H
#define AABBOX_H

#include <algorithm>

#include "Ray.hpp"

// Axis-aligned bounding box
class AABB
{
public:
    AABB(glm::vec3 bl_ = glm::vec3(), glm::vec3 tr_ = glm::vec3())
    {
        bl = bl_, tr = tr_;
    }

    // Expand to fit box
    void  expand(const AABB &box)
    {
        if (box.bl.x < bl.x) { bl.x = box.bl.x; }

        if (box.bl.y < bl.y) { bl.y = box.bl.y; }

        if (box.bl.z < bl.z) { bl.z = box.bl.z; }

        if (box.tr.x > tr.x) { tr.x = box.tr.x; }

        if (box.tr.y > tr.y) { tr.y = box.tr.y; }

        if (box.tr.z > tr.z) { tr.z = box.tr.z; }
    }

    // Expand to fit point
    void  expand(const glm::vec3 &vec)
    {
        if (vec.x < bl.x) { bl.x = vec.x; }

        if (vec.y < bl.y) { bl.y = vec.y; }

        if (vec.z < bl.z) { bl.z = vec.z; }
    }

    // Returns longest axis: 0, 1, 2 for x, y, z respectively
    int  get_longest_axis()
    {
        glm::vec3  diff = tr - bl;

        if ((diff.x > diff.y) && (diff.x > diff.z)) { return 0; }

        if ((diff.y > diff.x) && (diff.y > diff.z)) { return 1; }

        return 2;
    }

    // Check if ray intersects with box. Returns true/false and stores distance in t
    bool  intersection(const Ray &r, float &t)
    {
        float  tx1  = (bl.x - r.origin.x) * r.direction_inv.x;
        float  tx2  = (tr.x - r.origin.x) * r.direction_inv.x;
        float  tmin = std::min(tx1, tx2);
        float  tmax = std::max(tx1, tx2);
        float  ty1  = (bl.y - r.origin.y) * r.direction_inv.y;
        float  ty2  = (tr.y - r.origin.y) * r.direction_inv.y;

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        float  tz1 = (bl.z - r.origin.z) * r.direction_inv.z;
        float  tz2 = (tr.z - r.origin.z) * r.direction_inv.z;

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));
        t    = tmin > 0 ? tmin : tmax;

        return tmax >= tmin;
    }

private:
    glm::vec3  bl;    // Bottom left (min)
    glm::vec3  tr;    // Top right   (max)
};

#endif // AABBOX_H
