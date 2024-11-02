#include "Weapon.h"
#include "Entity.h"
#include "Player.h"

static const char* WEAPON_LIST[] = { "Pistol" };

Weapon *loadWeapon(const char *weaponFile) {
    SJson *weaponJson;
    weaponJson = sj_load(weaponFile);
    if (!weaponJson) {
        slog("No weapon file");
    }

    SJson *SJweaponName = sj_object_get_value(weaponJson, "Name");
    const char *weaponName = sj_get_string_value(SJweaponName);

    bool weaponExists = false;

    for (int i = 0; i < (sizeof(WEAPON_LIST) / sizeof(WEAPON_LIST[0])); i++) {
        const char* weaponName = WEAPON_LIST[i];
        if (strcmp(weaponName, (const char*) weaponName) == 0) {
            weaponExists = true;
        }
    }
    
    if (!weaponExists) {
        slog("Weapon does not exist.");
    }

    const char* weaponActor = malloc(strlen("actors/") + strlen(weaponName) + strlen(".actor") + 1);
    strcpy(weaponActor, "actors/");
    strcat(weaponActor, weaponName);
    strcat(weaponActor, ".actor");

    SJson *SJcartridgeSize = sj_object_get_value(weaponJson, "CartridgeSize");
    SJson *SJreloadSpeed = sj_object_get_value(weaponJson, "ReloadSpeed");
    SJson *SJmaxReserveAmmo = sj_object_get_value(weaponJson, "MaxReserveAmmo");
    SJson* SJammoType= sj_object_get_value(weaponJson, "AmmoType");
    SJson* SJdamage = sj_object_get_value(weaponJson, "Damage");
    
    int cartridgeSize, maxReserveAmmo, damage;
    float reloadSpeed; 
    const char* ammoType;
    sj_get_integer_value(SJcartridgeSize, &cartridgeSize);
    sj_get_float_value(SJreloadSpeed, &reloadSpeed);
    sj_get_integer_value(SJmaxReserveAmmo, &maxReserveAmmo);
    sj_get_integer_value(SJdamage, &damage);
    ammoType = malloc(strlen(sj_get_string_value(SJammoType)));
    strcpy(ammoType, sj_get_string_value(SJammoType));

    // Get weapon audio
    SJson* SJweaponUseSound = sj_object_get_value(weaponJson, "UseSound");
    const char* useSoundString = malloc(strlen("sounds/") + strlen(sj_get_string_value(SJweaponUseSound) + strlen(".wav")));
    strcpy(useSoundString, "sounds/");
    strcat(useSoundString, sj_get_string_value(SJweaponUseSound));
    strcat(useSoundString, ".wav");
    GFC_Sound* useSound = gfc_sound_load(useSoundString, 1.0, 0);

    
    Weapon* newWeapon = (Weapon*)malloc(sizeof(Weapon));
    newWeapon->name = weaponName;
    newWeapon->actorFile = weaponActor;
    newWeapon->cartridgeSize = cartridgeSize;
    newWeapon->reloadSpeed = reloadSpeed;
    newWeapon->maxReserveAmmo = maxReserveAmmo;
    newWeapon->currentAmmo = cartridgeSize;
    newWeapon->reserveAmmo = 40;
    strcpy(newWeapon->ammoType, ammoType);
    newWeapon->damage = damage;
    newWeapon->useSound = useSound;

    newWeapon->shoot = pistolFire;

    printf("\n%s successfully created!", newWeapon->name);

    return newWeapon;
}

Entity * shotCollided(GFC_Edge3D raycast, GFC_Box boundingBox) {
    Entity* hitEntity = NULL;
    GFC_Triangle3D t = { 0 };
    for (int i = 0; i < entityManager.entityMax; i++) {
        Entity * currEntity = &entityManager.entityList[i];
        // Get non-player entities

        if (!currEntity->_in_use) {
            continue;
        }
        // If the entity is not an enemy then skip
        if (currEntity->type != ENEMY) {
            continue;
        };
        //slog("Entity position: %f, %f, %f", entityManager.entityList[i].position.x, entityManager.entityList[i].position.y, entityManager.entityList[i].position.z);
        if (entityRaycastTest(currEntity, raycast, NULL, &t, &boundingBox)) {
            // If an entity has already been hit, then assign the new one only if it's closer
            if (!hitEntity) {
                hitEntity = currEntity;
            } else {
                if (gfc_vector3d_magnitude_between_squared(raycast.a, entityGlobalPosition(currEntity)) < gfc_vector3d_magnitude_between_squared(raycast.a, entityGlobalPosition(hitEntity))) {
                    hitEntity = currEntity;
                }
            }

        }
    }
    return hitEntity;
}

void pistolFire(Entity* self, Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition) {
    gfc_sound_play(weapon->useSound, 0, 0.2, 0, -1);
    GFC_Vector3D raycastStart = cameraPosition;
    GFC_Vector3D raycastAdd = gfc_vector3d(0, -1024, 32);
    gfc_vector3d_rotate_about_x(&raycastAdd, playerRotation.x);
    gfc_vector3d_rotate_about_z(&raycastAdd, playerRotation.z);
    GFC_Vector3D raycastVector = gfc_vector3d_added(raycastStart, raycastAdd);
    GFC_Edge3D raycast = gfc_edge3d_from_vectors(raycastStart, raycastVector);
    //slog("Raycast start: %f, %f, %f", raycastStart.x, raycastStart.y, raycastStart.z);
    //slog("Raycast end: %f, %f, %f", raycastAdd.x, raycastAdd.y, raycastAdd.z);

    //Uses a simple bounding box to filter out entities that cannoe possible be hit
    GFC_Box boundingBox = { 0 };
    boundingBox.x = min(self->position.x, raycastAdd.x) - 4;
    boundingBox.y = min(self->position.y, raycastAdd.y) - 4;
    boundingBox.z = min(self->position.z, raycastAdd.z) - 4;
    boundingBox.w = max(self->position.x, raycastAdd.x) - boundingBox.x + 4;
    boundingBox.d = max(self->position.y, raycastAdd.y) - boundingBox.y + 4;
    boundingBox.h = max(self->position.z, raycastAdd.z) - boundingBox.z + 4;

 


    PlayerData* playerData = (PlayerData*)self->data;
    playerData->boundingBoxTest = boundingBox;

    Entity* hitEntity = shotCollided(raycast, boundingBox);
    if (!hitEntity) {
        return;
    }
    entityAttacked(hitEntity, weapon->damage);

}