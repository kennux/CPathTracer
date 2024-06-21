#pragma once
#include "ptmath.h"
#include "material.h"
#include "scene.h"
#include "camera.h"

typedef struct TraceParameters
{
    int backbufferWidth;
    int backbufferHeight;

    BakedScene* scene;
    Camera* camera;

    int samplesPerPixel;
    mfloat maxDepth;
    int maxBounces;
} TraceParameters;

void trace(TraceParameters params, mfloat* backbuffer, long int* rayCount);