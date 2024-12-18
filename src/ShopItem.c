#include "ShopItem.h"
#include "simple_logger.h"
#include "Player.h"

Entity* shopItemNew(PurchaseableGuns gun) {
	Entity* shopItem = interactableNew(SHOP_ITEM, gfc_vector3d(0, 0, 0));
	if (!shopItem) {
		slog("Could not allocate shop item interactable.");
		return NULL;
	}

	ShopItemData *shopItemData = gfc_allocate_array(sizeof(ShopItemData), 1);
	if (!shopItemData) {
		slog("Could not allocate shop item data.");
		free(shopItem);
		return NULL;
	}

	Interactable* interactData = (Interactable*)shopItem->data;
	interactData->data = shopItemData;

	
	char* weaponName = "";

	switch (gun) {
	case MAGNUM:
		shopItem->model = gf3d_model_load("models/weapons/magnum.model");
		shopItemData->weaponFile = "GameData/WeaponData/Magnum.json";
		shopItemData->weaponPrice = 45;
		weaponName = "Magnum";
		break;
	case AUTO_SHOTGUN:
		shopItem->model = gf3d_model_load("models/weapons/auto_shotgun.model");
		shopItemData->weaponFile = "GameData/WeaponData/AutoShotgun.json";
		shopItemData->weaponPrice = 60;
		weaponName = "Auto Shotgun";
		break;
	case ASSAULT_RIFLE:
		shopItem->model = gf3d_model_load("models/weapons/assault_rifle.model");
		shopItemData->weaponFile = "GameData/WeaponData/AssaultRifle.json";
		shopItemData->weaponPrice = 70;
		weaponName = "Assault Rifle";
		break;
	case SMG:
		shopItem->model = gf3d_model_load("models/weapons/smg.model");
		shopItemData->weaponFile = "GameData/WeaponData/SMG.json";
		shopItemData->weaponPrice = 40;
		weaponName = "SMG";
		break;
	case MINIGUN:
		shopItem->model = gf3d_model_load("models/weapons/minigun.model");
		shopItemData->weaponFile = "GameData/WeaponData/Minigun.json";
		shopItemData->weaponPrice = 50;
		weaponName = "Minigun";
		break;
	case ROCKET_LAUNCHER:
		shopItem->model = gf3d_model_load("models/weapons/rocket_launcher.model");
		shopItemData->weaponFile = "GameData/WeaponData/RocketLauncher.json";
		shopItemData->weaponPrice = 100;
		weaponName = "Rocket Launcher";
		break;
	case CROSSBOW:
		shopItem->model = gf3d_model_load("models/weapons/crossbow.model");
		shopItemData->weaponFile = "GameData/WeaponData/Crossbow.json";
		shopItemData->weaponPrice = 60;
		weaponName = "Crossbow";
		break;
	}
	sprintf(interactData->interactText, "Buy %s: $%d", weaponName, shopItemData->weaponPrice);



	shopItem->update = shopItemUpdate;

	interactData->interact = shopItemInteract;
	
	return shopItem;
}


void shopItemInteract(Entity* player, Entity* entity, Interactable* interactData) {
	ShopItemData* shopItemData = (ShopItemData*)interactData->data;

	PlayerData* playerData = (PlayerData*)player->data;
	if (playerData->money < shopItemData->weaponPrice) {
		return;
	}

	playerData->money -= shopItemData->weaponPrice;
	giveWeapon(player, playerData, shopItemData->weaponFile);
	free(shopItemData);
	interactableDelete(entity);
}

void shopItemUpdate(Entity* self, float delta) {
	self->rotation.z += -1 * delta;
}