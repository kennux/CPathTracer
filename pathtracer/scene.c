//
// Created by kennu on 19/06/2024.
//

#include <stdbool.h>
#include "ptmath.h"
#include "scene.h"

int scene_Raycast(HitInfo* outHitInfo, Scene* scene, Ray* ray, mfloat minDist, mfloat maxDist)
{
    int hitCount = 0;
    HitInfo hitInfo;
    HitInfo localHitInfo;

    // Spheres
    for (size_t i = 0; i < scene->sphereCount; i++)
    {
        Sphere* sphere = &scene->spheres[i];

        Vec3f oc;
        // origin - center
        p_v3f_sub_v3f(&oc, &ray->origin, &sphere->center);

        // oc.direction
        mfloat b;
        p_v3f_dot(&b, &oc, &ray->direction);

        if (b > 0)
            continue;

        bool hasHit = false;

        // length sq of oc
        mfloat c;
        p_v3f_lengthSq(&c, &oc);

        mfloat discriminantSqr = b * b - (c - (sphere->radius * sphere->radius));
        if (discriminantSqr > 0)
        {
            mfloat discriminant = sqrt(discriminantSqr);
            localHitInfo.distance = (-b - discriminant);
            if (localHitInfo.distance < maxDist && localHitInfo.distance > minDist)
            {
                // Calculate hit point
                p_ray_getPoint(&localHitInfo.point, ray, localHitInfo.distance);

                // Calculate normal
                p_v3f_sub_v3f(&localHitInfo.normal, &localHitInfo.point, &sphere->center);
                p_v3f_mul_f(&localHitInfo.normal, &localHitInfo.normal, 1.0f / sphere->radius);

                // Set material
                localHitInfo.material = sphere->material;

                hasHit = true;
            }
            else
            {
                localHitInfo.distance = (-b + discriminant);
                if (localHitInfo.distance < maxDist && localHitInfo.distance > minDist)
                {
                    // Calculate hit point
                    p_ray_getPoint(&localHitInfo.point, ray, localHitInfo.distance);

                    // Calculate normal
                    p_v3f_sub_v3f(&localHitInfo.normal, &localHitInfo.point, &sphere->center);
                    p_v3f_mul_f(&localHitInfo.normal, &localHitInfo.normal, 1.0f / sphere->radius);

                    // Set material
                    localHitInfo.material = sphere->material;

                    hasHit = true;
                }
            }
        }

        if (hasHit)
        {
            if (hitCount == 0)
                hitInfo = localHitInfo;
            else
            {
                if (hitInfo.distance > localHitInfo.distance)
                    hitInfo = localHitInfo; // Better hit - exchange!
            }

            hitCount++;
        }
    }

    *outHitInfo = hitInfo;
    return hitCount;
}