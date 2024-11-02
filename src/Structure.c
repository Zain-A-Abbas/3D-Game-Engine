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

	newStructure->model = gf3d_model_load("models/structures/house.model");

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

	// Making a door
	Entity* testDoor = createDoor(newStructure);
	testDoor->position = gfc_vector3d(4, -20, 0);
	Entity* testPickup = createAmmoPickup(newStructure);
	testPickup->position = gfc_vector3d(0, 4, 0);

	return newStructure;
}