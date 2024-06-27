#pragma once
#include "ptmath.h"
#include "material.h"
#include "scene.h"
#include "camera.h"
#include <stdint.h>

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

typedef struct TraceTileParameters
{
    int xStart;
    int yStart;
    int regionWidth;
    int regionHeight;

    TraceParameters traceParams;
} TraceTileParameters;

TraceTileParameters singleTileTraceParams(TraceParameters params);
void parallelTileTraceParams(TraceParameters params, int tileSizeX, int tileSizeY, TraceTileParameters* outTileParams);
size_t parallelTileTraceParams_TileCount(TraceParameters params, int tileSizeX, int tileSizeY);

void traceParallel(TraceTileParameters* tiles, size_t tileCount, mfloat* backbuffer, uint64_t* rayCount, int threadCount);
void traceTile(TraceTileParameters tileParams, mfloat* backbuffer, uint64_t* rayCount);