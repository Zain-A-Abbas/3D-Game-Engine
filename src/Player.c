#include "Player.h"
#include "gf2d_mouse.h"
#include "Character3D.h"
#include "UI.h"
#include "Powerup.h"
#include "TypesExtra.h"
#include "light.h"

const Uint8 PLAYER_LAYERS = 0b10000000;

const float PLAYER_SPEED = 16;
const float HORIZONTAL_MOUSE_SENSITIVITY = 2.0;
const float VERTICAL_MOUSE_SENSITIVITY = 1.6;
const int MAX_RELATIVE_MOUSE_X = 10;
const int HIGHEST_X_DEGREES = 30;
const int LOWEST_X_DEGREES = -30;

const float INTERACT_DISTANCE = 8;

const GFC_Vector3D BASE_CAMERA_OFFSET = { -4, 26, 10 };
const GFC_Vector3D ZOOM_CAMERA_OFFSET = { -2.75, 14, 9.75 };
GFC_Vector3D actualCameraOffset;
GFC_Vector3D zoomCameraOffset;

GFC_Vector2D walkDirection = { 0 };

float speedMod = 1.0;
bool reloading = false;
bool aimZoom = false;

Entity * createPlayer() {
    Entity * playerEntity = entityNew();
    playerEntity->think = think;
    playerEntity->update = update;
  

    PlayerData * playerData = (PlayerData*) malloc(sizeof(PlayerData));
    if (!playerData) {
        slog("Failed to allocate memory for player data.");
        return NULL;
    }

    playerEntity->type = PLAYER;
    playerEntity->collisionLayer = PLAYER_LAYERS;

    memset(playerData, 0, sizeof(PlayerData));
    playerEntity->data = playerData;
    playerData->state = PS_FREE;
    playerData->hp = 100;
    playerData->playerWeapons = gfc_list_new();
    
    
    playerData->weaponsUnlocked = 0;
    playerData->ammo[0] = 0;
    playerData->ammo[1] = 0;
    playerData->ammo[2] = 20;
    giveWeapon(playerEntity, playerData, "GameData/WeaponData/Pistol.json");
    giveWeapon(playerEntity, playerData, "GameData/WeaponData/Shotgun.json");
    //giveWeapon(playerEntity, playerData, "GameData/WeaponData/Magnum.json");
    //giveWeapon(playerEntity, playerData, "GameData/WeaponData/AssaultRifle.json");
    //giveWeapon(playerEntity, playerData, "GameData/WeaponData/SMG.json");
    //giveWeapon(playerEntity, playerData, "GameData/WeaponData/AutoShotgun.json");
    //giveWeapon(playerEntity, playerData, "GameData/WeaponData/Minigun.json");
    //giveWeapon(playerEntity, playerData, "GameData/WeaponData/RocketLauncher.json");
    //giveWeapon(playerEntity, playerData, "GameData/WeaponData/Crossbow.json");

    playerData->reload = false;
    playerData->reloadTimer = 0.0;

    playerData->cameraTrauma = gfc_vector3d(0, 0, 0);
    playerData->cameraTraumaDecay = gfc_vector3d(0, 0, 0);
    actualCameraOffset = BASE_CAMERA_OFFSET;
    zoomCameraOffset = ZOOM_CAMERA_OFFSET;

    playerData->character3dData = newCharacter3dData();
    playerData->character3dData->gravityRaycastHeight = 3;

    playerData->attackCooldown = 0;

    GFC_Box boxTest = { 0 };
    playerData->boundingBoxTest = boxTest;

    // UI setup
    assignPlayer(playerData);

    // Model/animations

    //playerEntity->model = gf3d_model_load("models/dino.model");
    animationSetup(
        playerEntity,
        "models/player/", 
        (char *[]){
            "PlayerIdle",
            NULL
        }
    );
    animationPlay(playerEntity, "PlayerIdle", true);
    playerEntity->scale = gfc_vector3d(8, 8, 8);

    // Collision
    EntityCollision* collision = (EntityCollision*) malloc(sizeof(EntityCollision));
    memset(collision, 0, sizeof(EntityCollision));

    // Create capsule
    GFC_ExtendedPrimitive* collisionPrimitive = (GFC_ExtendedPrimitive*)malloc(sizeof(GFC_ExtendedPrimitive));
    memset(collisionPrimitive, 0, sizeof(GFC_ExtendedPrimitive));
    collisionPrimitive->type = E_Capsule;
    GFC_Capsule playerCapsule = gfc_capsule(8, 2);
    collisionPrimitive->s.c = playerCapsule;
    collision->collisionPrimitive = collisionPrimitive;
    
    GFC_Box boundingBox = {0};
    boundingBox.w = 8;
    boundingBox.d = 24;
    boundingBox.h = 8;

    collision->AABB = boundingBox;

    playerEntity->entityCollision = collision;

    return playerEntity;
}

PlayerData * getPlayerData(Entity * self){

    if (self == NULL || self->data == NULL) {
        return NULL;
    }
    return (PlayerData*) self->data;
}

void assignCamera(Entity * self, Camera * newCam) {
    PlayerData * playerData = getPlayerData(self);
    playerData->camera = newCam;
    gf3d_camera_look_at(self->position, NULL);
}

void playerFree(Entity * self) {
    PlayerData * playerData = getPlayerData(self);
    if (playerData){
        free(playerData);
    }
}

void think(Entity * self, float delta) {
    PlayerData * playerData = getPlayerData(self);
    for (int i = 0; i < 5; ++i) {
        if (playerData->powerups[i]) {
            playerData->powerupTimers[i] -= delta;
            //printf("\n%f", playerData->powerupTimers[i]);
            if (playerData->powerupTimers[i] <= 0.0) {
                playerData->powerups[i] = false;
                //slog("\nPowerup timed out.");
            }
        }
    }
    speedMod = 1.0 + 0.7 * playerData->powerups[SPEED_BOOST];
    if (playerData->reload) {
        reloadProcess(self, playerData, delta);
    }
    _playerControls(self, delta);
}


void update(Entity * self, float delta) {
    _playerUpdate(self, delta);
}   

void _playerControls(Entity * self, float delta) {
    PlayerData * playerData = getPlayerData(self);
    Character3DData* character3dData = playerData->character3dData;
    
    if (gfc_input_command_pressed("interact")) {
        //playerData->hp -= 10;
        interact(self, playerData);
    }

    // Horizontal movement
    GFC_Vector2D inputVector = gfc_vector2d(
        gfc_input_command_down("walkright") - gfc_input_command_down("walkleft"),
        gfc_input_command_down("walkforward") - gfc_input_command_down("walkback")
    );

    // Movement animations
    GFC_Vector2D tempWalkDirection = inputVector;
    if ((int)tempWalkDirection.x != 0 && (int)tempWalkDirection.y != 0) {
        tempWalkDirection.x = 0;
    }
    if (tempWalkDirection.x != walkDirection.x || tempWalkDirection.y != walkDirection.y) {
        walkDirection = tempWalkDirection;
        //printf("\nWalk direction: %f, %f", walkDirection.x, walkDirection.y);
        /*if (walkDirection.y == 1 && walkDirection.x == 0) {
            animationPlay(self, "models/player/PlayerWalkForward.model");
        }
        else {
            animationPlay(self, "models/player/PlayerIdle.model");
        }*/
    }

    inputVector = gfc_vector2d_rotate(inputVector, character3dData->rotation.z);

    GFC_Vector3D movementVelocity = gfc_vector3d(inputVector.x, inputVector.y, 0);

    movementVelocity.x *= PLAYER_SPEED;
    movementVelocity.y *= PLAYER_SPEED;
    movementVelocity.x *= -1;
    movementVelocity.y *= -1;

    playerData->character3dData->motionVelocity.x = movementVelocity.x;
    playerData->character3dData->motionVelocity.y = movementVelocity.y;
    
    // Mouse rotation
    int xRel, yRel;
    float mouseX, mouseY;
    SDL_GetRelativeMouseState(&xRel, &yRel);
    mouseX = xRel * 0.1;
    mouseY = yRel * 0.1;
    if (mouseX > 10) {
        mouseX = 10;
    } else if (mouseX < -10) {
        mouseX = -10;
    }

    if (aimZoom) {
        mouseX *= 0.7;
        mouseY *= 0.7;
    }

    playerData->character3dData->rotation.z -= mouseX * HORIZONTAL_MOUSE_SENSITIVITY * delta;
    playerData->character3dData->rotation.x += mouseY * VERTICAL_MOUSE_SENSITIVITY * delta;
    if (playerData->character3dData->rotation.x > HIGHEST_X_DEGREES * GFC_DEGTORAD) playerData->character3dData->rotation.x = HIGHEST_X_DEGREES * GFC_DEGTORAD;
    if (playerData->character3dData->rotation.x < LOWEST_X_DEGREES * GFC_DEGTORAD) playerData->character3dData->rotation.x = LOWEST_X_DEGREES * GFC_DEGTORAD;


    // Zoom
    aimZoom = gf2d_mouse_button_held(2);

    int weaponSwitch = (!playerData->reload) && (int)gfc_input_mouse_wheel_up() - (int)gfc_input_mouse_wheel_down();
    if (weaponSwitch) {
        //printf("\nWeapon count: %d", gfc_list_get_count(playerData->playerWeapons));
        //printf("\nCurrent weapon: %d", playerData->currentWeapon);
        weaponSwitch += playerData->currentWeapon;

        if (weaponSwitch >= (int)gfc_list_get_count(playerData->playerWeapons)) {

;            weaponSwitch = 0;
        }
        else if (weaponSwitch < 0) {
            weaponSwitch = gfc_list_get_count(playerData->playerWeapons) - 1;
        }
        setWeapon(playerData, weaponSwitch);

    }

    

    // Attacking
    Weapon* weaponData = (Weapon*)gfc_list_get_nth(playerData->playerWeapons, playerData->currentWeapon);
    if (gf2d_mouse_button_pressed(0) || (gf2d_mouse_button_held(0) && weaponData->automatic)) {
        attack(self, playerData, character3dData);
    }

    if (gf2d_mouse_button_held(0) && strcmp(weaponData->name, "Minigun") == 0) {
        speedMod *= 0.25;
    }


    if (gfc_input_command_pressed("reload")) {
        reload(self, playerData);
    }
    if (playerData->state != PS_FREE) {
        return;
    }
}

void _playerUpdate(Entity * self, float delta) {


    // Movement
    PlayerData * playerData = getPlayerData(self);
    
    Character3DData* character3dData = playerData->character3dData;

    character3dData->motionVelocity.x *= speedMod;
    character3dData->motionVelocity.y *= speedMod;

    moveAndSlide(self, character3dData, delta);


    playerData->attackCooldown = fMoveTowards(playerData->attackCooldown, 0, delta);



    // Zoom
    GFC_Vector3D cameraMove;
    if (aimZoom) {
        cameraMove = gfc_vector3d_subbed(actualCameraOffset, zoomCameraOffset);
    }
    else {
        cameraMove = gfc_vector3d_subbed(actualCameraOffset, BASE_CAMERA_OFFSET);
    }
    cameraMove = gfc_vector3d_multiply(cameraMove, gfc_vector3d(delta * 16, delta * 16, delta * 16));
    actualCameraOffset = gfc_vector3d_subbed(actualCameraOffset, cameraMove);


    // Mouse update
    if (playerData->camera) {

        // Gets the camera offset, rotates it around the player's Z and X rotations, then adds it to the player's position
        gf3d_camera_set_position(getCameraPosition(self));

        // Takes the base camera rotation, and adds together its Z rotation and the player's Z rotation
        GFC_Vector3D baseCamRotation = gfc_vector3d_added(CAMERA_ROTATION, gfc_vector3d(0, 0, character3dData->rotation.z));
        baseCamRotation = gfc_vector3d_added(baseCamRotation, gfc_vector3d(character3dData->rotation.x, 0, 0));
        baseCamRotation = gfc_vector3d_added(baseCamRotation, playerData->cameraTrauma);
        gf3d_camera_set_rotation(baseCamRotation);

        // Move the player model in the direction facing
        float targetRotation = self->rotation.z + (character3dData->rotation.z - self->rotation.z) * 0.1;
        self->rotation.z = targetRotation;
    }

    // Camera trauma reduction
    playerData->cameraTrauma.x = fMoveTowards(playerData->cameraTrauma.x, 0, playerData->cameraTraumaDecay.x * delta);
    playerData->cameraTrauma.y = fMoveTowards(playerData->cameraTrauma.y, 0, playerData->cameraTraumaDecay.y * delta);
    playerData->cameraTrauma.z = fMoveTowards(playerData->cameraTrauma.z, 0, playerData->cameraTraumaDecay.z * delta);

    if (playerData->cameraTrauma.x == 0) {
        playerData->cameraTraumaDecay.x = 0;
    }
    if (playerData->cameraTrauma.y == 0) {
        playerData->cameraTraumaDecay.y = 0;
    }
    if (playerData->cameraTrauma.z == 0) {
        playerData->cameraTraumaDecay.z = 0;
    }

    interactScan(self);

}

void interactScan(Entity* self) {
    GFC_Vector3D interactPoint = gfc_vector3d(0, -INTERACT_DISTANCE, 0);
    GFC_Vector3D interactPosition;
    Interactable* interactable = NULL;
    Interactable* tempInteractable = NULL;
    Entity* currEntity;
    PlayerData* playerData = getPlayerData(self);
    gfc_vector3d_rotate_about_z(&interactPoint, self->rotation.z);
    interactPoint = gfc_vector3d_added(interactPoint, entityGlobalPosition(self));

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Filter out inactive entities, non-interactables, and interactables out of range
        currEntity = &entityManager.entityList[i];
        if (!currEntity->_in_use) {
            continue;
        }

        if (currEntity->type != INTERACTABLE) {
            continue;
        }

        tempInteractable = (Interactable*)currEntity->data;
        interactPosition = gfc_vector3d_added(entityGlobalPosition(currEntity), tempInteractable->interactOrigin);
        if (!gfc_vector3d_distance_between_less_than(interactPoint, interactPosition, INTERACT_DISTANCE)) {
            continue;
        }
        interactable = tempInteractable;

    }

    playerData->currentInteractable = interactable;

}

void interact(Entity * self, PlayerData* playerData) {
    if (playerData->currentInteractable) {
        playerData->currentInteractable->interact(self, playerData->currentInteractable->interactEntity, playerData->currentInteractable);
    }
}

GFC_Vector3D getCameraPosition(Entity *self) {
    PlayerData * playerData = getPlayerData(self);
    Character3DData* character3dData = playerData->character3dData;
    GFC_Vector3D newCamPosition = actualCameraOffset;
    GFC_Vector3D temp = newCamPosition;
    gfc_vector3d_rotate_about_x(&temp, character3dData->rotation.x);
    newCamPosition = gfc_vector3d_added(newCamPosition, gfc_vector3d(temp.x - newCamPosition.x, -fabsf(temp.y - newCamPosition.y), temp.z - newCamPosition.z));
    gfc_vector3d_rotate_about_z(&newCamPosition, character3dData->rotation.z);
    //printf("\nRotated: %f, %f, %f", newCamPosition.x, newCamPosition.y, newCamPosition.z);


    newCamPosition = gfc_vector3d_added(newCamPosition, self->position);
    return newCamPosition;
}

void addCameraTrauma(PlayerData* playerData, GFC_Vector3D trauma, GFC_Vector3D traumaDecay) {
    if (playerData->powerups[NO_RECOIL]) {
        return;
    }
    if (gfc_vector3d_magnitude(trauma) > gfc_vector3d_magnitude(playerData->cameraTrauma)) {
        playerData->cameraTrauma = trauma;
        playerData->cameraTraumaDecay = traumaDecay;
    }
}

void attack(Entity * self, PlayerData * playerData, Character3DData * character3dData) {
    if (playerData->attackCooldown != 0) {
        return;
    }

    Weapon* weaponData = gfc_list_get_nth(playerData->playerWeapons, playerData->currentWeapon);
    if (weaponData->currentAmmo <= 0 && !playerData->powerups[INFINITE_AMMO]) {
        return;
    }

    GFC_Vector3D gunfirePosition = gfc_vector3d(0, -4, 0);
    gfc_vector3d_rotate_about_z(&gunfirePosition, self->rotation.z);
    gunfirePosition = gfc_vector3d_added(gunfirePosition, self->position);
    activateGunfireLight(gunfirePosition);

    if (!playerData->powerups[INFINITE_AMMO]) {
        weaponData->currentAmmo -= 1;
    }

    addCameraTrauma(playerData, gfc_vector3d(-0.08, 0, 0), gfc_vector3d(1.0, 0, 0));
    if (gf2d_mouse_button_held(2)) {
        playerData->cameraTrauma = gfc_vector3d_multiply(playerData->cameraTrauma, gfc_vector3d(0.5, 0.5, 0.5));
    }
    playerData->attackCooldown = weaponData->attackCooldown;
    weaponData->shoot(self, weaponData, self->position, character3dData->rotation, getCameraPosition(self));
}

void reload(Entity * self, PlayerData * playerData) {
    if (playerData->reload) {
        return;
    }
    
    Weapon* weaponData = gfc_list_get_nth(playerData->playerWeapons, playerData->currentWeapon);
    int ammoIndex = weaponData->reserveAmmoIndex;

    int reloadAmount = MIN(weaponData->cartridgeSize - weaponData->currentAmmo, MIN(weaponData->cartridgeSize, playerData->ammo[ammoIndex]));
    
    if (reloadAmount == 0) {
        return;
    }

    playerData->reload = true;
    playerData->reloadTimer = 2.0;
}

void reloadProcess(Entity * self, PlayerData * playerData, float delta) {
    playerData->reloadTimer -= delta + 3 * delta * playerData->powerups[QUARTER_RELOAD_TIME];
    printf("\nTimer: %f", playerData->reloadTimer);
    if (playerData->reloadTimer <= 0.0) {
        Weapon* weaponData = gfc_list_get_nth(playerData->playerWeapons, playerData->currentWeapon);
        int ammoIndex = weaponData->reserveAmmoIndex;

        int reloadAmount = MIN(weaponData->cartridgeSize - weaponData->currentAmmo, MIN(weaponData->cartridgeSize, playerData->ammo[ammoIndex]));
        playerData->reload = true;
        playerData->reloadTimer = 2.0;

        playerData->ammo[ammoIndex] -= reloadAmount;
        weaponData->currentAmmo += reloadAmount;
        playerData->reload = false;
    } 
}

void giveWeapon(Entity* self, PlayerData* playerData, const char *weapon) {
    int newWeaponIndex = playerData->weaponsUnlocked;
    playerData->weaponsUnlocked += 1;
    gfc_list_append(playerData->playerWeapons, loadWeapon(weapon, playerData));
    setWeapon(playerData, newWeaponIndex);
}

void setWeapon(PlayerData* playerData, int weaponIndex) {
    playerData->currentWeapon = weaponIndex;
    playerSwitchWeapon((Weapon*)gfc_list_get_nth(playerData->playerWeapons, weaponIndex));
}

void playerTakeDamage(Entity * self, int damage) {
    PlayerData *playerData = (PlayerData*) self->data;
    if (playerData->powerups[INVINCIBLE]) {
        return;
    }
    playerData->hp = MAX(playerData->hp - damage, 0);
}