#include "simple_logger.h"
#include "Interactable.h"
#include "Player.h"

const Uint8 INTERACTABLE_LAYERS = 0b00010000;



Entity* interactableNew(InteractableType type, GFC_Vector3D interactOffset) {

	Entity* newInteractable = entityNew();
	if (!newInteractable) {
		slog("Could not create interactable.");
		return NULL;
	}

	newInteractable->type = INTERACTABLE;
	switch (type) {
	case DOOR:
		newInteractable->model = gf3d_model_load("models/structures/shop_door.model");
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

	memset(interactable, 0, sizeof(interactable));
	interactable->interactEntity = newInteractable;
	newInteractable->collisionLayer = INTERACTABLE_LAYERS;
	newInteractable->data = interactable;

	interactable->canInteract = true;
	interactable->interact = baseInteract;
	interactable->interactOrigin = interactOffset;

	setInteractText(interactable, "aaa");
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

Entity* createShopEntryDoor() {
	Entity* door = interactableNew(DOOR, gfc_vector3d(0, 0, 0));
	if (!door) {
		slog("Could not create door.");
		return NULL;
	}
	door->position = gfc_vector3d(0, 8, -8);
	Interactable* doorData = (Interactable*)door->data;
	setInteractText(doorData, "Enter shop");
	doorData->interact = shopEntryDoorInteract;
	return door;
}

Entity* createShopExitDoor(Entity *shop) {
	Entity* door = interactableNew(DOOR, gfc_vector3d(0, 0, 0));
	door->parent = shop;
	if (!door) {
		slog("Could not create door.");
		return NULL;
	}
	door->position = gfc_vector3d(0, 28, 0);
	Interactable* doorData = (Interactable*)door->data;
	setInteractText(doorData, "Exit shop");
	doorData->interact = shopExitDoorInteract;
	return door;
}

void shopEntryDoorInteract(Entity* player, Entity* entity, Interactable* interactData) {
	PlayerData* playerData = (PlayerData*)player->data;
	playerData->preShopPosition = player->position;
	player->position = gfc_vector3d(0, 0, 1001);
	player->rotation = gfc_vector3d(0, 0, 0);
	playerData->character3dData->rotation = gfc_vector3d(0, 0, 0);
}

void shopExitDoorInteract(Entity* player, Entity* entity, Interactable* interactData) {
	PlayerData* playerData = (PlayerData*)player->data;
	player->position = playerData->preShopPosition;
}