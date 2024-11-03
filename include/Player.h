#ifndef __PLAYER__
#define __PLAYER__

#include "simple_logger.h"
#include "Entity.h"
#include "gfc_input.h"
#include "gf3d_camera.h"
#include "gf3d_draw.h"
#include "Character3D.h"
#include "Interactable.h"
#include "Weapon.h"

#define FAR_CAMERA_OFFSET gfc_vector3d(-8, 32, 4)
#define CAMERA_ROTATION gfc_vector3d(M_PI, 0, M_PI)

typedef enum {
    PS_NONE,
    PS_FREE,
    PS_SHOP
} PlayerState;

typedef struct PlayerData_S {
    Camera              *camera;    // Pointer to camera
    int                 hp;
    PlayerState         state;
    GFC_List            *playerWeapons;
    int                 currentWeapon;
    int                 weaponsUnlocked;
    int                 ammo[4]; // 0 is handgun, 1 is shotgun, 2 is AR, 3 is rocket
    float               attackCooldown;
    GFC_Vector3D        cameraTrauma;
    GFC_Vector3D        cameraTraumaDecay;
    Character3DData     *character3dData;
    Interactable        *currentInteractable;


    GFC_Box             boundingBoxTest;
    GFC_List            *raycastTests;

} PlayerData;


/**
 * @brief Creates an entity and assigns the player functions to it.
 */
Entity * createPlayer();

/**
 * @brief Assigns a camera to the player.
 */
void assignCamera(Entity * self, Camera * newCam);

void think(Entity * self, float delta);
void update(Entity * self, float delta);
void playerFree(Entity * self);

void _playerControls(Entity * self, float delta);
void _playerUpdate(Entity * self, float delta);

/**
* @brief Gets the player data.
*/
PlayerData* getPlayerData(Entity* self);

/**
 * @brief Returns where the camera position should be
 */
GFC_Vector3D getCameraPosition(Entity *self);

/**
* @brief Adds the trauma to the camera rotation every frame.
* @param The trauma value. Added on to the player's current camera truma, so if used again before it runs out it will stack.
* @parma the amount by which the trama decreases every second. Only the latest traumaDecay is used, rather than it stacking.
*/
void addCameraTrauma(PlayerData* playerData, GFC_Vector3D trauma, GFC_Vector3D traumaDecay);

/**
* @brief Checks all objects in an area in front of the player to see if they can be interacted with.
*/
void interactScan(Entity* self);

/**
* @brief Interacts with the current interactable object set by interactScan
*/
void interact(Entity * self, PlayerData* playerData);

/**
* @brief Triggered when the player hits the attack button.
*/
void attack(Entity* self, PlayerData* playerData, Character3DData* character3dData);

void reload(Entity* self, PlayerData* playerData);

/**
* @brief Sets the current weapon to the following one in the array. Also updates the UI.
*/
void setWeapon(PlayerData* playerData, int weaponIndex);

/**
* @brief Gives the player a weapon, initializes it at the base ammo, and then switches to it.
*/
void giveWeapon(Entity* self, PlayerData* playerData, const char* weapon);

#endif