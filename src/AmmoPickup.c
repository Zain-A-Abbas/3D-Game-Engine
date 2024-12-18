#include "AmmoPickup.h"
#include "simple_logger.h"
#include "TypesExtra.h"
#include "Player.h"

const float AMMO_ACCELERATION = 10.0;

Entity* createAmmoPickup(Entity* player) {
	Entity* newPickup = entityNew();

	AmmoPickupData* pickupData = (AmmoPickupData*)malloc(sizeof(AmmoPickupData));
	if (!pickupData) {
		slog("Could not create pickup data");
		_entityFree(newPickup);
		return NULL;
	}

	memset(pickupData, 0, sizeof(pickupData));
	
	newPickup->model = gf3d_model_load("models/interactables/ammo_pickup.model");
	
	pickupData->ammoSpeed = 20.0;
	pickupData->ammoType = gfc_random_int(3);
	printf("\nAmmo type: %d\n", pickupData->ammoType);
	pickupData->player = player;

	newPickup->data = pickupData;
	newPickup->update = ammoPickupUpdate;
	
	return newPickup;
}



void ammoPickupUpdate(struct Entity_S* self, float delta) {
	self->rotation.z += delta;

	AmmoPickupData* pickupData = (AmmoPickupData*) self->data;
	pickupData->ammoSpeed += delta * AMMO_ACCELERATION;
	Entity* player = pickupData->player;
	GFC_Vector3D targetPosition = gfc_vector3d(player->position.x, player->position.y, player->position.z + 4);
	GFC_Vector3D pickupVelocity = gfc_vector3d_subbed(targetPosition, self->position);
	gfc_vector3d_normalize(&pickupVelocity);
	pickupVelocity.x *= pickupData->ammoSpeed * delta; pickupVelocity.y *= pickupData->ammoSpeed * delta; pickupVelocity.z *= pickupData->ammoSpeed * delta;
	self->position = gfc_vector3d_added(self->position, pickupVelocity);

	if (!gfc_vector3d_distance_between_less_than(self->position, targetPosition, 1)) {
		return;	
	}

	PlayerData* playerData = (PlayerData*)player->data;
	Weapon* weapon;
	int i = 0;
	int ammoUsed = 0;
	for (i = 0; i < gfc_list_get_count(playerData->playerWeapons); i++) {
		weapon = (Weapon*)gfc_list_get_nth(playerData->playerWeapons, i);
		if (weapon->reserveAmmoIndex == pickupData->ammoType) {
			playerData->ammo[weapon->reserveAmmoIndex] = MIN(playerData->ammo[weapon->reserveAmmoIndex] + 8, 999);
			ammoUsed = 1;
			break;
		}
	}

	if (ammoUsed) {
		free(pickupData);
		_entityFree(self);
	}
}