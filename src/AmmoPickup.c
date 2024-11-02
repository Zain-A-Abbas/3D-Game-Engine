#include "AmmoPickup.h"
#include "simple_logger.h"
#include "TypesExtra.h"
#include "Player.h"



Entity* createAmmoPickup(Entity* parent) {
	Entity* newPickup = interactableNew(AMMO_PICKUP, gfc_vector3d(0, 0, 0));
	newPickup->parent = parent;
	newPickup->update = NULL;


	Interactable* pickupInteractable = (Interactable*)newPickup->data;
	pickupInteractable->interact = ammoPickupInteract;

	AmmoPickupData* pickupData = (AmmoPickupData*)malloc(sizeof(AmmoPickupData));
	if (!pickupData) {
		slog("Could not create pickup data");
		free(pickupInteractable);
		_entityFree(newPickup);
		return NULL;
	}
	memset(pickupData, 0, sizeof(pickupData));
	strcpy(pickupData->ammoType, "Pistol");

	pickupInteractable->data = pickupData;
	setInteractText(pickupInteractable, "Pick up ammo.");
	
	return newPickup;
}



void ammoPickupInteract(Entity* player, Entity* entity, Interactable* interact) {
	PlayerData* playerData = (PlayerData*)player->data;
	Weapon* weapon;
	AmmoPickupData* ammoPickupData = (AmmoPickupData*)interact->data;
	int i = 0;
	int ammoUsed;
	for (i = 0; i < gfc_list_get_count(playerData->playerWeapons); i++) {
		weapon = (Weapon*)gfc_list_get_nth(playerData->playerWeapons, i);
		if (strcmp(weapon->ammoType, ammoPickupData->ammoType) == 0) {
			weapon->reserveAmmo = min(weapon->reserveAmmo + 8, weapon->maxReserveAmmo);
			ammoUsed = 1;
		}
	}

	if (ammoUsed) {
		interactableDelete(entity);
	}
}