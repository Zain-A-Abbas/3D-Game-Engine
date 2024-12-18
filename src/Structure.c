#include "simple_logger.h"
#include "Structure.h"
#include "Interactable.h"
#include "Door.h"
#include "AmmoPickup.h"

const Uint8 STRUCTURE_LAYERS = 0b00001010;


Entity* structureNew(StructureType type) {

	Entity* newStructure = entityNew();
	if (!newStructure) {
		slog("Could not create structure.");
		return NULL;
	}

	newStructure->type = STRUCTURE;
	newStructure->model = gf3d_model_load("models/structures/shop.model");

	StructureData* structureData = (StructureData*)malloc(sizeof(StructureData));
	if (!structureData) {
		slog("Could not create StructureData");
		free(newStructure);
		return NULL;
	}

	newStructure->collisionLayer = STRUCTURE_LAYERS;
	memset(structureData, 0, sizeof(structureData));
	newStructure->data = structureData;

	structureData->structureType = type;

	newStructure->free = structureFree;

	return newStructure;
}

void *structureFree(struct Entity_S* self) {
	if (self->data) {
		StructureData* structureData = (StructureData*)self->data;
		gf3d_model_free(structureData->structureCollision);
		free(self->data);
	}
	_entityFree(self);
}