#ifndef __TERRAIN_MANAGER__
#define __TERRAIN_MANAGER__

#include "Entity.h"

typedef struct TerrainData_S {
	Model* terrainCollision;
} TerrainData;

/**
* @brief Creates an entity and assigns it the proper collision layer.
*/
Entity * terrainEntityNew();
void *terrainFree(struct Entity_S* self);

#endif