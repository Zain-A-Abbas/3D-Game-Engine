#ifndef __AMMO_PICKUP__
#define __AMMO_PICKUP__
#include "Interactable.h"
#include "Weapon.h"



typedef struct {
	int		ammoType;
} AmmoPickupData;

Entity* createAmmoPickup(Entity* parent, int ammoType);

void ammoPickupInteract(Entity* player, Entity* entity, Interactable* interact);

#endif