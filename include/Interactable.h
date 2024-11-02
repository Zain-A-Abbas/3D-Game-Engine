#ifndef __INTERACTABLE__
#define __INTERACTABLE__
#include "Entity.h"

typedef enum {
	NO_INTERACT,
	SPINNING_BOX,
	DOOR,
	AMMO_PICKUP
} InteractableType;

typedef struct Interactable_S {
	InteractableType		interactableType;
	Uint8					canInteract;
	GFC_Vector3D			interactOrigin;
	Entity					*interactEntity;
	void (*interact)		(Entity* player, Entity *entity, struct Interactable_S *interact);
	void*					data;	// Custom interactable data
	char					interactText[32];
} Interactable;

Entity* interactableNew(InteractableType type, GFC_Vector3D interactOffset);

void interactableDelete(Entity* self);

void setInteractText(Interactable* interactable, char* interactText);

void _interact(Entity *player, Entity* entity, Interactable* interactData);

void baseInteract(Entity* player, Entity* entity, Interactable* interactData);

#endif