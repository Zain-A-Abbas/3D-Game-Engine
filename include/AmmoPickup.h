#ifndef __AMMO_PICKUP__
#define __AMMO_PICKUP__
#include "Interactable.h"
#include "Weapon.h"


typedef struct {
	int			ammoType;
	float		ammoSpeed;
	Entity		*player;
} AmmoPickupData;

Entity* createAmmoPickup(Entity *player);

void ammoPickupUpdate(struct Entity_S* self, float delta);

#endif