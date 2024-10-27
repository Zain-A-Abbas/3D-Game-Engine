#ifndef __INTERACTABLE__
#define __INTERACTABLE__
#include "Entity.h"

typedef enum {
	NO_INTERACT,
	SPINNING_BOX,
	DOOR
} InteractableType;

typedef struct Interactable_S {
	InteractableType		interactableType;
	Uint8					canInteract;
	GFC_Vector3D			interactOrigin;
	void (*interact)		(Entity *entity, struct Interactable_S *interact);
	void*					data;	// Custom interactable data
} Interactable;

Entity* interactableNew(InteractableType type, GFC_Vector3D interactOffset);


void _interact(Entity* entity, Interactable* interactData);

void baseInteract(Entity* entity, Interactable* interactData);

#endif