#ifndef __WEAPON__
#define __WEAPON__

#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"
#include "stdbool.h"
#include <string.h>
#include "gf2d_actor.h"
#include "gfc_audio.h"
#include "Entity.h"
#include "Player.h"

typedef struct PlayerData_S PlayerData;

// Determines the ammo types
static const char* AMMO_TYPES[] = { 
    "Handgun",
    "Shotgun",
    "AR",
    "Rocket"
};

typedef struct Weapon_S
{
    char*     name;
    char*     actorFile;
    char*     modelFile;
    int             cartridgeSize;
    float           attackCooldown;
    float           reloadTime;
    int             currentAmmo;
    int             reserveAmmoIndex;
    int             damage;
    float           spreadDegrees;
    Uint8           automatic;
    GFC_Sound*      useSound;
    // Behavior
    void (*shoot)   (
        Entity          *player,
        struct Weapon_S * weapon,
        GFC_Vector3D playerPosition,
        GFC_Vector3D playerRotation,
        GFC_Vector3D cameraPosition
        ); // The weapon's fire function
} Weapon;

/**
 * @brief Loads the weapon and returns the struct.
 */
Weapon* loadWeapon(const char* weaponFile, PlayerData* playerData);

/**
* @brief Returns an entity hit by a raycast. Does not work if the entity's origin is more than 2 meters away from the edge of its collision
*/
Entity* shotCollided(GFC_Edge3D raycast, GFC_Box *boundingBox);
/**
 * @brief Pistol's fire function.
 */
void singleFire(Entity* self, Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition);

/**
 * @brief Shotgun's fire function.
 */
void shotgunFire(Entity* self, Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition);

/**
* @brief Projectile's fire function.
*/
void projectileFire(Entity* self, Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition);

#endif