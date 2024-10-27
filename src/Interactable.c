#include "simple_logger.h"
#include "Interactable.h"

const Uint8 INTERACTABLE_LAYERS = 0b00001010;



Entity* interactableNew(InteractableType type, GFC_Vector3D interactOffset) {

	Entity* newInteractable = entityNew();
	if (!newInteractable) {
		slog("Could not create interactable.");
		return NULL;
	}

	newInteractable->type = INTERACTABLE;
	newInteractable->model = gf3d_model_load("models/interactables/door.model");

	Interactable* interactable = (Interactable*)malloc(sizeof(Interactable));
	if (!interactable) {
		slog("Could not create Interactable");
		free(newInteractable);
		return NULL;
	}

	newInteractable->collisionLayer = INTERACTABLE_LAYERS;
	memset(interactable, 0, sizeof(interactable));
	newInteractable->data = interactable;

	interactable->canInteract = true;
	interactable->interact = baseInteract;
	interactable->interactOrigin = interactOffset;
	return newInteractable;
}

void _interact(Entity* entity, Interactable* interactData) {
	if (!interactData) return;
	if (interactData->interact && interactData->canInteract) {
		interactData->interact(entity, interactData);
		return;
	} else {
		slog("Interactable does not have interact function pointer set");
	}
}

void baseInteract(Entity* entity, Interactable* interactData) {
	entity->rotation.z += M_PI / 16;
}

