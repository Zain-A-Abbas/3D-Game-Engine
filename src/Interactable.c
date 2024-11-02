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
	switch (type) {
	case DOOR:
		newInteractable->model = gf3d_model_load("models/interactables/door.model");
		break;
	case AMMO_PICKUP:
		newInteractable->model = gf3d_model_load("models/interactables/ammo_pickup.model");
		break;
	default:
		break;
	}

	Interactable* interactable = (Interactable*)malloc(sizeof(Interactable));
	if (!interactable) {
		slog("Could not create Interactable");
		free(newInteractable);
		return NULL;
	}

	interactable->interactEntity = newInteractable;
	newInteractable->collisionLayer = INTERACTABLE_LAYERS;
	memset(interactable, 0, sizeof(interactable));
	newInteractable->data = interactable;

	interactable->canInteract = true;
	interactable->interact = baseInteract;
	interactable->interactOrigin = interactOffset;
	return newInteractable;
}
void interactableDelete(Entity* self) {
	if (self->type != INTERACTABLE) {
		return;
	}
	Interactable* data = (Interactable*)self->data;
	free(data);
	_entityFree(self);
}

void setInteractText(Interactable* interactable, char* interactText) {
	if (strlen(interactText) > sizeof(interactable->interactText)) {
		return;
	}
	memset(interactable->interactText, 0, sizeof(interactable->interactText));
	strcpy(interactable->interactText, interactText);
}

void _interact(Entity* player, Entity* entity, Interactable* interactData) {
	if (!interactData) return;
	if (interactData->interact && interactData->canInteract) {
		interactData->interact(player, entity, interactData);
		return;
	} else {
		slog("Interactable does not have interact function pointer set");
	}
}

void baseInteract(Entity* player, Entity* entity, Interactable* interactData) {
	entity->rotation.z += M_PI / 16;
}

