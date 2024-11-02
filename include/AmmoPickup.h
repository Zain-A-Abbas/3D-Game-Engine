#ifndef __AMMO_PICKUP__
#define __AMMO_PICKUP__
#include "Interactable.h"
#include "Weapon.h"



typedef struct {
	char	ammoType[16];
} AmmoPickupData;

Entity* createAmmoPickup(Entity* parent);

void ammoPickupInteract(Entity* player, Entity* entity, Interactable* interact);

#endif