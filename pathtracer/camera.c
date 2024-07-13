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


THREAD_LOCAL SIMD_ALIGN Vec3f_Pack packOffset1, packOffset2, packOffset, packRayOrigin, packRayDir;
THREAD_LOCAL SIMD_ALIGN AlignedFloatPack rndMulLensU, rndMulLensV;

void camera_GetRays(Ray *outRays, size_t rayCount, Camera* camera, float u, float v, RandomState* rndState)
{
    Vec3f horizontalU, verticalV, rayDirBase;
    p_v3f_mul_f(&horizontalU, &camera->horizontal, u);
    p_v3f_mul_f(&verticalV, &camera->vertical, v);
    p_v3f_add_v3f(&rayDirBase, &camera->lowerLeftCorner, &horizontalU);
    p_v3f_add_v3f(&rayDirBase, &rayDirBase, &verticalV);
    p_v3f_sub_v3f(&rayDirBase, &rayDirBase, &camera->origin);

    for (size_t i = 0; i < rayCount; i += SIMD_MATH_WIDTH)
    {
        Vec3f_Pack randomVec;
        random_packInUnitDisk(&randomVec, rndState);

        // rndMulLens = vecX|vecY * lensRadius
        sip_f_mul_ps(&rndMulLensU, &randomVec.x, camera->lensRadius);
        sip_f_mul_ps(&rndMulLensV, &randomVec.y, camera->lensRadius);

        // u|v * rndMulLens
        sip_vf_add_sp(&packOffset1, &camera->u, &rndMulLensU);
        sip_vf_add_sp(&packOffset2, &camera->v, &rndMulLensV);

        // offset = offset1 + offset2
        sip_v_add_pp(&packOffset, &packOffset1, &packOffset2);

        // rayOrigin = camOrigin + offset
        sip_v_add_sp(&packRayOrigin, &camera->origin, &packOffset);

        // rayDirBase - offset
        sip_v_sub_sp(&packRayDir, &rayDirBase, &packOffset);

        sip_v_normalizeUnsafe_p(&packRayDir, &packRayDir);

        #pragma clang loop unroll(full)
        for (size_t j = 0; j < SIMD_MATH_WIDTH; j++)
        {
            size_t rayIdx = i + j;

            if (rayIdx >= rayCount)
                break; // We're done

            Ray* outRay = &outRays[rayIdx];

            sip_v_extract_s(&packRayDir, &outRay->direction, j);
            sip_v_extract_s(&packRayOrigin, &outRay->origin, j);
        }
    }
}

void camera_GetRay(Ray *outRay, Camera* camera, float u, float v, RandomState* rndState)
{
    Vec3f vec;
    random_inUnitDisk(&vec, rndState);

    Vec3f offset1, offset2, offset;
    // offset1 = u * vecX * lensRadius
    p_v3f_mul_f(&offset1, &camera->u, vec.x * camera->lensRadius);
    // offset2 = v * vecX * lensRadius
    p_v3f_mul_f(&offset2, &camera->v, vec.y * camera->lensRadius);

    // offset = offset1 + offset2
    p_v3f_add_v3f(&offset, &offset1, &offset2);

    // rayOrigin = camOrigin + offset
    p_v3f_add_v3f(&outRay->origin, &camera->origin, &offset);

    outRay->direction.x = (camera->lowerLeftCorner.x + u * camera->horizontal.x + v * camera->vertical.x - camera->origin.x - offset.x);
    outRay->direction.y = (camera->lowerLeftCorner.y + u * camera->horizontal.y + v * camera->vertical.y - camera->origin.y - offset.y);
    outRay->direction.z = (camera->lowerLeftCorner.z + u * camera->horizontal.z + v * camera->vertical.z - camera->origin.z - offset.z);
    p_v3f_normalize(&outRay->direction, &outRay->direction);
}