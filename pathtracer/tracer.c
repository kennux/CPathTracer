//
// Created by kennu on 19/06/2024.
//

#include <time.h>
#include "tracer.h"

void trace(TraceParameters params, mfloat* backbuffer)
{
    random_state state = time(NULL);

    mfloat invWidth = 1.0f / params.backbufferWidth;
    mfloat invHeight = 1.0f / params.backbufferHeight;
    mfloat invSamplesPerPixel = 1.0f / (mfloat)params.samplesPerPixel;

    // RGB
    float colors[3];

    for (int x = 0; x < params.backbufferWidth; x++)
    {
        for (int y = 0; y < params.backbufferHeight; y++)
        {
            colors[0] = colors[1] = colors[2] = 0;
            mfloat u = x * invWidth, v = y * invHeight;
            int colorIndex = (y * params.backbufferWidth + x) * 4;

            for (int r = 0; r < params.samplesPerPixel; r++)
            {
                // Get ray
                Ray ray;
                camera_GetRay(&ray, params.camera, u, v, &state);

                // Trace ray
                HitInfo hitInfo;
                int hits = scene_Raycast(&hitInfo, params.scene, &ray, 0.01, params.maxDepth);
                if (hits > 0)
                {
                    backbuffer[colorIndex] = 1;
                    backbuffer[colorIndex + 1] = 0;
                    backbuffer[colorIndex + 2] = 0;
                    backbuffer[colorIndex + 3] = 1;
                }
                else
                {
                    backbuffer[colorIndex] = 0;
                    backbuffer[colorIndex + 1] = 0;
                    backbuffer[colorIndex + 2] = 0;
                    backbuffer[colorIndex + 3] = 1;
                }
            }
        }
    }
}