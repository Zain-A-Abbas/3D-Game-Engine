#ifndef __DOOR__
#define __DOOR__
#include "Interactable.h"

typedef enum {
	CLOSED,
	OPEN,
	CLOSING,
	OPENING,
	BROKEN
} DoorState;

typedef struct {
	DoorState		doorState;
} DoorData;

Entity* createDoor(Entity *parent);

void doorUpdate(Entity* self, float delta);

void doorInteract(Entity* entity, Interactable* interact);

#endif