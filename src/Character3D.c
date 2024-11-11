#include "Character3D.h"
#include "simple_logger.h"
#include "Player.h"

const float BASE_GRAVITY = -1;
const float BASE_HORIZONTAL_COLLISION_RADIUS = 2;

Character3DData* newCharacter3dData() {
    Character3DData* newData = (Character3DData*) malloc (sizeof(Character3DData));
    if (!newData) {
        slog("Could not create Character3DData.");
        return NULL;
    }

    memset(newData, 0, sizeof(newData));

    newData->velocity = gfc_vector3d(0, 0, 0);
    newData->gravity = BASE_GRAVITY;
    newData->rotation = gfc_vector3d(M_PI, 0, 0);
    newData->zSnap = 0;

    newData->horizontalCollisionRadius = BASE_HORIZONTAL_COLLISION_RADIUS;

    return newData;
}

void moveAndSlide(Entity* self, Character3DData* character3dData) {
    self->position.x += character3dData->velocity.x;
    self->position.y += character3dData->velocity.y;
    self->position.z += character3dData->velocity.z;
    if (character3dData->zSnap) {
        //slog("Pos z: %f", self->position.z);
        //slog("Snap z: %f", snapZ);
        self->position.z = character3dData->zSnapTarget;
        character3dData->zSnap = false;
    }

}

void horizontalWallSlide(Entity* self, Character3DData* character3dData, float delta) {
    GFC_Vector3D velocity = character3dData->velocity;
    GFC_Vector3D intersectionPoint = { 0 };
    GFC_Vector3D penetrationNormal = { 0 };
    float penetrationDepth = 0;

    PlayerData* playerData = NULL;
    /*if (self->type == PLAYER) {
        playerData = (PlayerData*)self->data;
        gfc_list_clear(playerData->raycastTests);
        playerData->raycastTests = gfc_list_new_size(8);
    }*/

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Filter out inactive entities, non-collideable, and collideable out of range
        Entity* currEntity = &entityManager.entityList[i];
        if (!currEntity->_in_use) {
            continue;
        }
        if (!isOnLayer(currEntity, 3) && !isOnLayer(currEntity, 2)) {
            continue;
        }
        if (!gfc_vector3d_distance_between_less_than(entityGlobalPosition(self), entityGlobalPosition(currEntity), character3dData->horizontalCollisionRadius * 24)) {
            continue;
        }
        
        if (entityCapsuleTest(currEntity, self->entityCollision->collisionPrimitive->s.c, &intersectionPoint, &penetrationNormal, &penetrationDepth, NULL)) {
            GFC_Vector3D pushBack = penetrationNormal;

            pushBack.x *= penetrationDepth;// *0.01;
            pushBack.y *= penetrationDepth;// *0.01;
            pushBack.z *= penetrationDepth;// *0.01;
            velocity = gfc_vector3d_added(velocity, pushBack);
        }

    }
    character3dData->velocity = velocity;
}


GFC_Vector3D verticalVectorMovement(Entity * self, Character3DData * character3dData, float delta) {
    GFC_Vector3D velocity = character3dData->velocity;
    GFC_Vector3D intersectionPoint = { 0 };
    GFC_Vector3D penetrationNormal = { 0 };
    float penetrationDepth = 0;

    Bool isOnFloor = false;

    PlayerData* playerData = NULL;
    /*if (self->type == PLAYER) {
        playerData = (PlayerData*)self->data;
        gfc_list_clear(playerData->raycastTests);
        playerData->raycastTests = gfc_list_new_size(8);
    }*/

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Filter out inactive entities, non-collideable, and collideable out of range
        Entity* currEntity = &entityManager.entityList[i];
        if (!currEntity->_in_use) {
            continue;
        }
        if (!isOnLayer(currEntity, 1)) {
            continue;
        }


        if (entityCapsuleTest(currEntity, self->entityCollision->collisionPrimitive->s.c, &intersectionPoint, &penetrationNormal, &penetrationDepth, NULL)) {
            float floorAngle = M_PI / 2 - asinf(penetrationNormal.z);
            // Make it snap to a certain height when traversing on ground with a lower angle than on the previous frame to account for slightly dipping into the floor
            if (character3dData->previousFloorAngle > floorAngle) {
                character3dData->zSnapTarget = intersectionPoint.z;
            }

            character3dData->previousFloorAngle = floorAngle;

            // Get the normalized non-vertical movement
            GFC_Vector3D horizontalDirection = gfc_vector3d(character3dData->velocity.x, character3dData->velocity.y, 0);
            gfc_vector3d_normalize(&horizontalDirection);
            // Used to determine if moving up or down
            float dotProduct = gfc_vector3d_dot_product(penetrationNormal, horizontalDirection);
            float horizontalMagnitude = sqrtf(pow(character3dData->velocity.x, 2) + pow(character3dData->velocity.y, 2));

            // Get the dot product as if parallel to the slope, to slow down vertical movement in case the player is not moving parallel to it
            GFC_Vector3D opposite = gfc_vector3d(-penetrationNormal.x, -penetrationNormal.y, 0);
            gfc_vector3d_normalize(&opposite);
            float parallelDotProduct = gfc_vector3d_dot_product(penetrationNormal, opposite);

            // Uses tanf rather than sinf as the horizontal speed should not adjust with the angle of the slope
            float floorRatio = tanf(floorAngle) * fabs(dotProduct / parallelDotProduct);
            

            if (dotProduct < 0) {
                velocity.z = horizontalMagnitude * floorRatio;
            }
            else if (dotProduct > 0) {
                velocity.z = -horizontalMagnitude * floorRatio;
            }
            else {
                velocity.z = 0;
            }
            //printf("\n%f", penetrationNormal.z);
            isOnFloor = true;

        }

    }


    //character3dData->zSnap = 0;
    if (!isOnFloor) {
        character3dData->zSnap = 0;
        velocity.z += character3dData->gravity * delta;
    }

    character3dData->velocity = velocity;

}

int isOnFloor(Entity* self, Character3DData* character3dData, GFC_Vector3D* floorNormal, GFC_Vector3D* contact) {
    GFC_Triangle3D t = { 0 };
    GFC_Vector3D gravityRaycastDir;
    GFC_Edge3D gravityRaycast;

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Get ground entitiesentities
        if (!entityManager.entityList[i]._in_use) {
            continue;
        }
        if (!isOnLayer(&entityManager.entityList[i], 1)) {
            continue;
        }

        // Found terrain
        for (int j = 0; j < 4; j++) {
            gravityRaycastDir = gfc_vector3d(0.125, 0, character3dData->gravityRaycastHeight);
            gfc_vector3d_rotate_about_z(&gravityRaycastDir, j * GFC_HALF_PI);
            gravityRaycast = gfc_edge3d_from_vectors(gfc_vector3d_added(entityGlobalPosition(self), gravityRaycastDir), entityGlobalPosition(self));
            if (entityRaycastTest(&entityManager.entityList[i], gravityRaycast, contact, &t, NULL)) {
                /*printf("%f, %f, %f", t.a.x, t.a.y, t.a.z);
                printf("%f, %f, %f", t.b.x, t.b.y, t.b.z);
                printf("%f, %f, %f", t.c.x, t.c.y, t.c.z);*/
                *floorNormal = gfc_trigfc_angle_get_normal(t);
                return true;
            }
        }

    }
    return false;
}