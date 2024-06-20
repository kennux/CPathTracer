//
// Created by kennu on 18/06/2024.
//
#include "camera.h"
#include "random.h"

Camera camera_Construct(Vec3f lookFrom, Vec3f lookAt, Vec3f vup, float vfov, float aspect, float aperture, float focusDist)
{
    Camera camera;
    camera.lensRadius = aperture / 2.0f;
    float theta = vfov * PI / 180.0f;
    float halfHeight = tan(theta / 2.0f);
    float halfWidth = aspect * halfHeight;

    camera.origin = lookFrom;
    camera.w = v3f_normalize(v3f_sub_v3f(lookFrom, lookAt));
    camera.u = v3f_normalize(v3f_cross(vup, camera.w));
    camera.v = v3f_cross(camera.w, camera.u);

    // ((halfWidth * focusDist) * u)
    Vec3f scaledU = v3f_mul_f(camera.u, halfWidth * focusDist);

    // ((halfHeight * focusDist) * v)
    Vec3f scaledV = v3f_mul_f(camera.v, halfHeight * focusDist);

    // (focusDist * w)
    Vec3f scaledW = v3f_mul_f(camera.w, focusDist);

    camera.lowerLeftCorner = lookFrom;
    camera.lowerLeftCorner = v3f_sub_v3f(camera.lowerLeftCorner, scaledU);
    camera.lowerLeftCorner = v3f_sub_v3f(camera.lowerLeftCorner, scaledV);
    camera.lowerLeftCorner = v3f_sub_v3f(camera.lowerLeftCorner, scaledW);

    // 2 * halfWidth * focusDist * u
    camera.horizontal = v3f_mul_f(camera.u, 2 * halfWidth * focusDist);
    // 2 * halfHeight * focusDist * v
    camera.vertical = v3f_mul_f(camera.v, 2 * halfHeight * focusDist);

    return camera;
}

void camera_GetRay(Ray *outRay, Camera* camera, float u, float v, RandomState* rndState)
{
    Vec3f vec;
    random_in_unit_disk(&vec, rndState);

    Vec3f offset1, offset2, offset;
    p_v3f_mul_f(&offset1, &camera->u, vec.x * camera->lensRadius);
    p_v3f_mul_f(&offset2, &camera->v, vec.y * camera->lensRadius);
    p_v3f_add_v3f(&offset, &offset1, &offset2);

    p_v3f_add_v3f(&outRay->origin, &camera->origin, &offset);

    outRay->direction.x = (camera->lowerLeftCorner.x + u * camera->horizontal.x + v * camera->vertical.x - camera->origin.x - offset.x);
    outRay->direction.y = (camera->lowerLeftCorner.y + u * camera->horizontal.y + v * camera->vertical.y - camera->origin.y - offset.y);
    outRay->direction.z = (camera->lowerLeftCorner.z + u * camera->horizontal.z + v * camera->vertical.z - camera->origin.z - offset.z);
    p_v3f_normalize(&outRay->direction, &outRay->direction);
}