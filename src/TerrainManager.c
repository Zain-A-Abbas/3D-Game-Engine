#include "simple_logger.h"
#include "gfc_matrix.h"
#include "TerrainManager.h"

const Uint8 TERRAIN_LAYERS = 0b00000010;

Entity * terrainEntityNew() {
	Entity* terrainEntity = entityNew();
	if (terrainEntity == NULL) {
		slog("Terrain Entity could not be created");
		return NULL;
	}
	terrainEntity->type = TERRAIN;
	terrainEntity->collisionLayer = TERRAIN_LAYERS;

	TerrainData *newData = (TerrainData*)malloc(sizeof(TerrainData));
	memset(newData, 0, sizeof(TerrainData));
	newData->terrainCollision = NULL;
	terrainEntity->data = newData;
	
	terrainEntity->free = terrainFree;

	return terrainEntity;
}

void *terrainFree(struct Entity_S* self) {
	if (self->data) {
		TerrainData* terrainData = (TerrainData*)self->data;
		gf3d_model_free(terrainData->terrainCollision);
		free(self->data);
	}
	_entityFree(self);
}