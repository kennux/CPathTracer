#include "scene_internal.h"

void _raycast_ExchangeHit(HitInfo* currentBest, HitInfo* hitInfo, int hitCount)
{
    if (hitCount == 0)
        *currentBest = *hitInfo; // First hit
    else {
        if (currentBest->distance > hitInfo->distance)
            *currentBest = *hitInfo; // Better hit - exchange!
    }
}