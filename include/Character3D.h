#ifndef __CHARACTER_3D__
#define __CHARACTER_3D__

#include "Entity.h"
#include "stdbool.h"

/* This file handles 3D collisions and updates on entities meant to represent individual characters. 
Not suited for unique circumstances such as tiny swarms. */

// Used for checking collision layers
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

typedef struct  {
    GFC_Vector3D        rotation; // Used if the character needs a separate rotation for gameplay purposes than entity visuals
    GFC_Vector3D        motionVelocity; // Gotten from inputs/AI
    GFC_Vector3D        gravityVelocity; // Acted upon the enemy
    GFC_Vector3D        velocity; // Composited every frame from the above two
    GFC_Vector3D        appliedVelocity; // Forced applied every frame
    GFC_Vector3D        appliedVelocityDamp; // Is subtracted from appliedVelocity every frame until appliedVelocity is 0,0,0
    float               previousFloorAngle; // Vertical angle of the floor the character was on last frame
    float               gravityRaycastHeight; // The difference between the entity's origin point and the end of the "is on floor" raycast
    int                 zSnap; // Determines if the Z position should snap the next frame
    float               zSnapTarget; // The Z position to snap to next frame
    float               gravity; // Automatically set to negative 1 on creation
    float               horizontalCollisionRadius; // The character's collision radius
    GFC_Edge3D          gravityRaycast;
    bool                isOnFloor;
} Character3DData;

/**
 * @brief Creates a Character3DData structure.
 */
Character3DData* newCharacter3dData();

/**
* @Brief moves the character while sliding them across walls.
*/
void moveAndSlide(Entity* self, Character3DData* character3dData, float delta);

void collide(Entity* self, Character3DData* character3dData, float delta, bool* isOnFloor, GFC_Vector3D* collisionPushback, GFC_Capsule capsule, bool floorCollisions);

/**
* @brief Adjusts the speed and direction of the character if pushing against a wall
*/
void horizontalWallSlide(Entity* self, Character3DData* character3dData, float delta);
/**
* @brief Adjusts the vertical movement based on the slope.
* @param The character entity.
* @param The entity's Character3DData.
* @param The time passed since the last frame.
*/
GFC_Vector3D verticalVectorMovement(Entity* self, Character3DData* character3dData, float delta);

/**
* @brief Checks if 3d character is on ground or not.
* @param The character entity.
* @param The entity's Character3DData.
* @param The angle of the ground (if on ground).
* @param The point in coordinate space where the contact was detected.
*/
int isOnFloor(Entity* self, Character3DData* character3dData, GFC_Vector3D* floorNormal, GFC_Vector3D* contact);

#endif