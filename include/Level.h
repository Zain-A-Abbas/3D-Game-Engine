#ifndef __LEVEL__
#define __LEVEL__

#include "gf3d_model.h"
#include "Entity.h"

typedef struct {
    Model           *sky;
    GFC_Matrix4     skyMat;
    Entity          *terrain;
    GFC_List        *generatedTerrain;
} LevelLayout;

typedef struct {
    LevelLayout     *layout;
    GFC_List        *enemies;
    
} LevelData;

LevelData *createForestLevel(Entity **player);

void *levelDraw(LevelData *levelData);

#endif