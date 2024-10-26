#ifndef __CHARACTER_3D__
#define __CHARACTER_3D__

#include "Entity.h"

/* This file handles 3D collisions and updates on entities meant to represent individual characters. 
Not suited for unique circumstances such as tiny swarms. */

typedef struct  {
    GFC_Vector3D        rotation; // Used if the character needs a separate rotation for gameplay purposes than entity visuals
    GFC_Vector3D        velocity; // Applied to position every frame
    float               previousFloorAngle; // Vertical angle of the floor the character was on last frame
    float               gravityRaycastHeight; // The difference between the entity's origin point and the end of the "is on floor" raycast
    int                 zSnap; // Determines if the Z position should snap the next frame
    float               zSnapTarget; // The Z position to snap to next frame
    float               gravity; // Automatically set to negative 1 on creation
    float               horizontalCollisionRadius; // The character's collision radius
} Character3DData;

/**
 * @brief Creates a Character3DData structure.
 */
Character3DData* newCharacter3dData();

/**
* @Brief moves the character while sliding them across walls.
*/
void moveAndSlide(Entity* self, Character3DData* character3dData);

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