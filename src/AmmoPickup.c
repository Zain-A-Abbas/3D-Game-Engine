#include "AmmoPickup.h"
#include "simple_logger.h"
#include "TypesExtra.h"
#include "Player.h"



Entity* createAmmoPickup(Entity* parent, int ammoType) {
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
	pickupData->ammoType = 0;

	pickupInteractable->data = pickupData;
	setInteractText(pickupInteractable, "Pick up ammo.");
	
	return newPickup;
}



void ammoPickupInteract(Entity* player, Entity* entity, Interactable* interact) {
	PlayerData* playerData = (PlayerData*)player->data;
	Weapon* weapon;
	AmmoPickupData* ammoPickupData = (AmmoPickupData*)interact->data;
	int i = 0;
	int ammoUsed = 0;
	for (i = 0; i < gfc_list_get_count(playerData->playerWeapons); i++) {
		weapon = (Weapon*)gfc_list_get_nth(playerData->playerWeapons, i);
		if (weapon->reserveAmmoIndex == ammoPickupData->ammoType) {
			playerData->ammo[weapon->reserveAmmoIndex] = MIN(playerData->ammo[weapon->reserveAmmoIndex] + 8, 999);
			ammoUsed = 1;
			break;
		}
	}

	if (ammoUsed) {
		interactableDelete(entity);
	}
}