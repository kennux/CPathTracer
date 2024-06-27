#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "tracer.h"

TraceTileParameters singleTileTraceParams(TraceParameters params)
{
    TraceTileParameters p;
    p.regionHeight = params.backbufferHeight;
    p.regionWidth = params.backbufferWidth;
    p.xStart = 0;
    p.yStart = 0;
    p.traceParams = params;

    return p;
}

void local_parallelTileTraceParams_TileCount(TraceParameters params, int tileSizeX, int tileSizeY, int* tileCountX, int* tileCountY)
{
    *tileCountX = (int)ceil((double)params.backbufferWidth / (double)tileSizeX);
    *tileCountY = (int)ceil((double)params.backbufferHeight / (double)tileSizeY);
}

size_t parallelTileTraceParams_TileCount(TraceParameters params, int tileSizeX, int tileSizeY)
{
    int tileCountX, tileCountY;
    local_parallelTileTraceParams_TileCount(params, tileSizeX, tileSizeY, &tileCountX, &tileCountY);
    return tileCountX * tileCountY;
}

void parallelTileTraceParams(TraceParameters params, int tileSizeX, int tileSizeY, TraceTileParameters* outTileParams)
{
    int tileCountX, tileCountY;
    local_parallelTileTraceParams_TileCount(params, tileSizeX, tileSizeY, &tileCountX, &tileCountY);

    for (size_t x = 0; x < tileCountX; x++)
        for (size_t y = 0; y < tileCountY; y++)
        {
            size_t tileIdx = x * tileCountY + y;

            int xStart = x * tileSizeX;
            int yStart = y * tileSizeY;
            int xEnd = xStart + tileSizeX;
            int yEnd = yStart + tileSizeY;

            xEnd = min(xEnd, params.backbufferWidth);
            yEnd = min(yEnd, params.backbufferHeight);

            TraceTileParameters tileParameters;
            tileParameters.traceParams = params;
            tileParameters.xStart = xStart;
            tileParameters.yStart = yStart;
            tileParameters.regionWidth = xEnd - xStart;
            tileParameters.regionHeight = yEnd - yStart;
            tileParameters.traceParams = params;

            outTileParams[tileIdx] = tileParameters;
        }
}

typedef struct local_traceWorkerParams
{
    TraceTileParameters* tiles;
    size_t tileCount;
    mfloat* backbuffer;

    uint64_t* rayCount;
    LONG* jobPtr;
} local_traceWorkerParams;

DWORD WINAPI local_traceWorker(LPVOID arg)
{
    local_traceWorkerParams* params = (local_traceWorkerParams*)arg;
    LONG jobId = InterlockedAdd(params->jobPtr, 1);
    while (jobId < params->tileCount)
    {
        traceTile(params->tiles[jobId], params->backbuffer, params->rayCount);
        jobId = InterlockedAdd(params->jobPtr, 1);
    }

    return 0;
}

void traceParallel(TraceTileParameters* tiles, size_t tileCount, mfloat* backbuffer, uint64_t* rayCount, int threadCount, progressCallbackFunc progressCallback)
{
    HANDLE* threads = malloc(threadCount * sizeof(HANDLE));
    uint64_t* rayCounts = malloc(threadCount * sizeof(uint64_t));
    local_traceWorkerParams* params = malloc(threadCount * sizeof(local_traceWorkerParams));
    LONG jobPtr = -1;

    for (int i = 0; i < threadCount; ++i) {
        local_traceWorkerParams p;
        p.tileCount = tileCount;
        p.backbuffer = backbuffer;
        p.tiles = tiles;
        p.rayCount = &rayCounts[i];
        p.jobPtr = &jobPtr;

        params[i] = p;
        threads[i] = CreateThread(NULL, 0, local_traceWorker, &params[i], 0, NULL);
        rayCounts[i] = 0;
    }

    while (true)
    {
        DWORD waitResult = WaitForMultipleObjects(threadCount, threads, TRUE, 1000);
        if (waitResult >= WAIT_OBJECT_0 && waitResult < WAIT_OBJECT_0 + threadCount)
            break; // Exit the loop when all threads have finished

        progressCallback((float)jobPtr / tileCount);
    }

    *rayCount = 0;
    for (int i = 0; i < threadCount; ++i) {
        CloseHandle(threads[i]);
        *rayCount += rayCounts[i];
    }

    free(threads);
    free(rayCounts);
    free(params);
}

void traceTile(TraceTileParameters tileParams, mfloat* backbuffer, uint64_t* rayCount)
{
    TraceParameters  params = tileParams.traceParams;
    RandomState rand = time(NULL);

    mfloat invWidth = 1.0f / params.backbufferWidth;
    mfloat invHeight = 1.0f / params.backbufferHeight;
    mfloat invSamplesPerPixel = 1.0f / (mfloat)params.samplesPerPixel;

    // RGB
    Vec3f color;
    Ray ray;

    int endX = tileParams.xStart + tileParams.regionWidth;
    int endY = tileParams.yStart + tileParams.regionHeight;

    for (int x = tileParams.xStart; x < endX; x++)
    {
        for (int y = tileParams.yStart; y < endY; y++)
        {
            color = vec3f(0,0,0);
            mfloat u = x * invWidth, v = y * invHeight;
            int colorIndex = (y * params.backbufferWidth + x) * 4;

            for (int r = 0; r < params.samplesPerPixel; r++)
            {
                // Get ray
                camera_GetRay(&ray, params.camera, u, v, &rand);

                Vec3f localColor = params.scene->ambientLight;
                size_t bounceCount = 0;
                while (bounceCount < params.maxBounces)
                {
                    // Trace ray
                    HitInfo hitInfo;
                    (*rayCount)++;
                    int hits = scene_Raycast(&hitInfo, params.scene, &ray, 0.01, params.maxDepth);
                    if (hits > 0 && hitInfo.matIdx < params.scene->materials.materialCount)
                    {
                        Vec3f attenuation;
                        int scatter = material_Scatter(&hitInfo, &params.scene->materials, hitInfo.matIdx, &attenuation, &ray, &rand);
                        p_v3f_mul_v3f(&localColor, &attenuation, &localColor);

                        if (scatter == 0)
                            break;
                    }
                    else
                        break;

                    bounceCount++;
                }

                p_v3f_mul_f(&localColor, &localColor, invSamplesPerPixel);
                p_v3f_add_v3f(&color, &localColor, &color);
            }

            backbuffer[colorIndex + 0] = color.x;
            backbuffer[colorIndex + 1] = color.y;
            backbuffer[colorIndex + 2] = color.z;
            backbuffer[colorIndex + 3] = 1;
        }
    }
}