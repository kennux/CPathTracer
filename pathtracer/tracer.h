//
// Created by kennu on 19/06/2024.
//

#include "ptmath.h"
#include "material.h"
#include "scene.h"
#include "camera.h"

#ifndef CPATHTRACER_TRACER_H
#define CPATHTRACER_TRACER_H

typedef struct HitInfo
{
    Vec3f point;
    Vec3f normal;
    float distance;

    Material* material;
} HitInfo;

typedef struct TraceParameters
{
    int width;
    int height;

    Scene* scene;
    Camera* camera;

    int samplesPerPixel;
    mfloat maxDepth;
    int maxBounces;
} TraceParameters;

void trace(TraceParameters params, mfloat* backbuffer);

#endif //CPATHTRACER_TRACER_H
