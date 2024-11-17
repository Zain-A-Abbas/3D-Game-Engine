#include "Weapon.h"
#include "Entity.h"
#include "gfc_types.h"
#include "Projectile.h"


const int PISTOL_RANGE = 256;
const int SHOTGUN_RANGE = 32;
const int ASSAULT_RIFLE_RANGE = 256;
const int SMG_RANGE = 64;
const int ROCKET_SPEED = 256;

Weapon *loadWeapon(const char *weaponFile, PlayerData *playerData) {
    SJson *weaponJson;
    weaponJson = sj_load(weaponFile);
    if (!weaponJson) {
        slog("No weapon file");
    }

    SJson *SJweaponName = sj_object_get_value(weaponJson, "Name");
    const char *weaponName = sj_get_string_value(SJweaponName);


    const char* weaponActor = malloc(strlen("actors/") + strlen(weaponName) + strlen(".actor") + 1);
    strcpy(weaponActor, "actors/");
    strcat(weaponActor, weaponName);
    strcat(weaponActor, ".actor");

    SJson* SJattackCooldown = sj_object_get_value(weaponJson, "AttackCooldown");
    SJson *SJcartridgeSize = sj_object_get_value(weaponJson, "CartridgeSize");
    SJson *SJreloadTime = sj_object_get_value(weaponJson, "ReloadTime");

    SJson* SJammoType = sj_object_get_value(weaponJson, "AmmoType");
    SJson* SJModel = sj_object_get_value(weaponJson, "Model");
    SJson* SJdamage = sj_object_get_value(weaponJson, "Damage");
    SJson* SJspread= sj_object_get_value(weaponJson, "Spread");
    SJson* SJauto = sj_object_get_value(weaponJson, "Auto");
    SJson* SJspeed = sj_object_get_value(weaponJson, "Speed");
    
    int cartridgeSize, damage;
    int automatic;
    float reloadTime, attackCooldown, spread, speed = 0;
    const char* ammoType;
    const char* model = NULL;
    sj_get_integer_value(SJcartridgeSize, &cartridgeSize);
    sj_get_float_value(SJattackCooldown, &attackCooldown);
    sj_get_float_value(SJreloadTime, &reloadTime);
    sj_get_float_value(SJspread, &spread);
    sj_get_integer_value(SJdamage, &damage);
    sj_get_integer_value(SJauto, &automatic);
    ammoType = malloc(strlen(sj_get_string_value(SJammoType)));
    strcpy(ammoType, sj_get_string_value(SJammoType));
    if (SJModel) {
        model = malloc(strlen(sj_get_string_value(SJModel)));
        strcpy(model, sj_get_string_value(SJModel));
    }
    if (SJspeed) {
        sj_get_float_value(SJspeed, &speed);
    }

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
    newWeapon->attackCooldown = attackCooldown;
    newWeapon->reloadTime = reloadTime;
    newWeapon->currentAmmo = cartridgeSize;
    newWeapon->damage = damage;
    newWeapon->spreadDegrees = spread;
    newWeapon->automatic = automatic;
    newWeapon->projectileSpeed = speed;
    newWeapon->useSound = useSound;
    newWeapon->modelFile = model;


    for (int i = 0; i < sizeof(playerData->ammo) / sizeof(playerData->ammo[0]); i++) {
        if (strcmp(ammoType, AMMO_TYPES[i]) == 0) {
            newWeapon->reserveAmmoIndex = i;
            printf("\nAmmo found!");
        }
    }

    newWeapon->shoot = singleFire;
    if (strcmp(weaponName, "Shotgun") == 0) {
        newWeapon->shoot = shotgunFire;
    }
    else if (strcmp(weaponName, "Auto Shotgun") == 0) {
        newWeapon->shoot = shotgunFire;
    }
    else if (strcmp(weaponName, "Rocket Launcher") == 0) {
        newWeapon->shoot = projectileFire;
    }
    else if (strcmp(weaponName, "Crossbow") == 0) {
        newWeapon->shoot = projectileFire;
    }

    //printf("\n%s successfully created!", newWeapon->name);

    return newWeapon;
}

Entity * shotCollided(GFC_Edge3D raycast, GFC_Box *boundingBox) {
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
        if (!isOnLayer(currEntity, 2)) {
            continue;
        }
        //slog("Entity position: %f, %f, %f", entityManager.entityList[i].position.x, entityManager.entityList[i].position.y, entityManager.entityList[i].position.z);
        if (entityRaycastTest(currEntity, raycast, NULL, &t, boundingBox)) {
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

void singleFire(Entity* self, Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition) {
    gfc_sound_play(weapon->useSound, 0, 0.2, 0, -1);
    GFC_Vector3D raycastStart = cameraPosition;
    GFC_Vector3D raycastAdd = gfc_vector3d(0, -PISTOL_RANGE, 0);
    gfc_vector3d_rotate_about_x(&raycastAdd, playerRotation.x + gfc_crandom() * weapon->spreadDegrees * GFC_DEGTORAD);
    gfc_vector3d_rotate_about_z(&raycastAdd, playerRotation.z + gfc_crandom() * weapon->spreadDegrees * GFC_DEGTORAD);
    GFC_Vector3D raycastVector = gfc_vector3d_added(raycastStart, raycastAdd);
    GFC_Edge3D raycast = gfc_edge3d_from_vectors(raycastStart, raycastVector);
    //slog("Raycast start: %f, %f, %f", raycastStart.x, raycastStart.y, raycastStart.z);
    //slog("Raycast end: %f, %f, %f", raycastAdd.x, raycastAdd.y, raycastAdd.z);

    //Uses a simple bounding box to filter out entities that cannoe possibly be hit
    GFC_Box boundingBox = { 0 };
    boundingBox.x = MIN(self->position.x, raycastAdd.x) - 4;
    boundingBox.y = MIN(self->position.y, raycastAdd.y) - 4;
    boundingBox.z = MIN(self->position.z, raycastAdd.z) - 4;
    boundingBox.w = MAX(self->position.x, raycastAdd.x) - boundingBox.x + 4;
    boundingBox.d = MAX(self->position.y, raycastAdd.y) - boundingBox.y + 4;
    boundingBox.h = MAX(self->position.z, raycastAdd.z) - boundingBox.z + 4;


    PlayerData* playerData = (PlayerData*)self->data;
    playerData->boundingBoxTest = boundingBox;
    
    gfc_list_clear(playerData->raycastTests);
    playerData->raycastTests = gfc_list_new_size(8);

    GFC_Edge3D* testRaycast = (GFC_Edge3D*)malloc(sizeof(GFC_Edge3D));
    memset(testRaycast, 0, sizeof(testRaycast));
    testRaycast->a = raycast.a;
    testRaycast->b = raycast.b;
    gfc_list_append(playerData->raycastTests, testRaycast);


    Entity* hitEntity = shotCollided(raycast, &boundingBox);
    if (!hitEntity) {
        return;
    }

    entityAttacked(hitEntity, weapon->damage);
}

void shotgunFire(Entity* self, Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition) {
    gfc_sound_play(weapon->useSound, 0, 0.2, 0, -1);
    GFC_Vector3D raycastStart = cameraPosition;
    GFC_Vector3D raycastAdd = gfc_vector3d(0, -SHOTGUN_RANGE, 0);
    gfc_vector3d_rotate_about_x(&raycastAdd, playerRotation.x);
    gfc_vector3d_rotate_about_z(&raycastAdd, playerRotation.z);
    GFC_Vector3D raycastVector;
    GFC_Edge3D raycast;

    PlayerData* playerData = (PlayerData*)self->data;
    gfc_list_clear(playerData->raycastTests);
    playerData->raycastTests = gfc_list_new_size(8);

    GFC_Box boundingBox = { 0 };
    boundingBox.x = MIN(self->position.x, raycastAdd.x) - 16;
    boundingBox.y = MIN(self->position.y, raycastAdd.y) - 16;
    boundingBox.z = MIN(self->position.z, raycastAdd.z) - 8;
    boundingBox.w = MAX(self->position.x, raycastAdd.x) - boundingBox.x + 16;
    boundingBox.d = MAX(self->position.y, raycastAdd.y) - boundingBox.y + 16;
    boundingBox.h = MAX(self->position.z, raycastAdd.z) - boundingBox.z + 8;

    int i = 0;
    for (i = 0; i < 8; i++) {
        raycastAdd = gfc_vector3d(0, -SHOTGUN_RANGE, 0);
        gfc_vector3d_rotate_about_x(&raycastAdd, playerRotation.x + gfc_crandom() * weapon->spreadDegrees * GFC_DEGTORAD);
        gfc_vector3d_rotate_about_z(&raycastAdd, playerRotation.z + gfc_crandom() * weapon->spreadDegrees * GFC_DEGTORAD);
        raycastVector = gfc_vector3d_added(raycastStart, raycastAdd);
        raycast = gfc_edge3d_from_vectors(raycastStart, raycastVector);

        GFC_Edge3D* testRaycast = (GFC_Edge3D*)malloc(sizeof(GFC_Edge3D));
        memset(testRaycast, 0, sizeof(testRaycast));
        testRaycast->a = raycast.a;
        testRaycast->b = raycast.b;
        gfc_list_append(playerData->raycastTests, testRaycast);
        
            
        Entity* hitEntity = shotCollided(raycast, &boundingBox);
        if (!hitEntity) {
            continue;
        }
        entityAttacked(hitEntity, weapon->damage);

    }
}

void projectileFire(Entity* self, Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition) {
    Projectile* data = (Projectile*)malloc(sizeof(Projectile));
    PlayerData* playerData = (PlayerData*)self->data;
    data->damage = weapon->damage;
    data->layers = 0b00000110;
    data->maxLifetime = 4.0;
    data->lifetime = 0.0;
    data->velocity = gfc_vector3d(0, -weapon->projectileSpeed, 0);
    gfc_vector3d_rotate_about_x(&data->velocity, M_PI - playerData->camera->rotation.x);
    gfc_vector3d_rotate_about_z(&data->velocity, playerRotation.z);
    Entity * projectile = newProjectile(data, weapon->modelFile);
    if (!projectile) {
        return;
    }

    GFC_Vector3D normalizedVelocity = data->velocity;
    gfc_vector3d_normalize(&normalizedVelocity);
    projectile->rotation.y = -sinf(normalizedVelocity.z);
    projectile->rotation.z = playerRotation.z;
    GFC_Vector3D projectilePosition = cameraPosition;
    projectile->position = projectilePosition;
    
    // Keep projectile invisible until X time has passed
    projectile->scale = gfc_vector3d(0, 0, 0);

    // Make projectile raycast

    GFC_Vector3D raycastStart = {0};
    GFC_Vector3D raycastEnd = { 0 };
    data->raycast = gfc_edge3d_from_vectors(raycastStart, raycastEnd);
}