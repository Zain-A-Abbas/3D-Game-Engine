#ifndef __SHOP_ITEM__
#define __SHOP_ITEM__

#include "Interactable.h"

typedef enum {
	MAGNUM,
	ASSAULT_RIFLE,
	SMG,
	AUTO_SHOTGUN,
	MINIGUN,
	ROCKET_LAUNCHER,
	CROSSBOW
} PurchaseableGuns;

typedef struct {
	int			weaponPrice;
	char		*weaponFile;
} ShopItemData;

Entity* shopItemNew(PurchaseableGuns gun);

void shopItemInteract(Entity* player, Entity* entity, Interactable* interactData);

void shopItemUpdate(Entity* self, float delta);


#endif