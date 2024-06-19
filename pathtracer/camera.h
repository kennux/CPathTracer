//
// Created by kennu on 18/06/2024.
//

#include "ptmath.h"
#include "random.h"

#ifndef CPATHTRACER_CAMERA_H
#define CPATHTRACER_CAMERA_H

typedef struct Camera {
    Vec3f origin;
    Vec3f lowerLeftCorner;
    Vec3f horizontal;
    Vec3f vertical;
    Vec3f u, v, w;
    float lensRadius;
} Camera;

Camera camera_Construct(Vec3f lookFrom, Vec3f lookAt, Vec3f vup, float vfov, float aspect, float aperture, float focusDist);
void camera_GetRay(Ray *outRay, Camera* camera, float u, float v, random_state* rndState);

#endif //CPATHTRACER_CAMERA_H
