//
// Created by kennu on 18/06/2024.
//

#pragma once
#include "ptmath.h"
#include "random.h"

typedef struct Camera {
    Vec3f origin;
    Vec3f lowerLeftCorner;
    Vec3f horizontal;
    Vec3f vertical;
    Vec3f u, v, w;
    float lensRadius;
} Camera;

Camera camera_Construct(Vec3f lookFrom, Vec3f lookAt, Vec3f vup, float vfov, float aspect, float aperture, float focusDist);
void camera_GetRay(Ray *outRay, Camera* camera, float u, float v, RandomState* rndState);
