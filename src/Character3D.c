#include "Character3D.h"
#include "simple_logger.h"
#include "Player.h"
#include "Debugger.h"

const float BASE_GRAVITY = -98;
const float BASE_HORIZONTAL_COLLISION_RADIUS = 2;


Character3DData* newCharacter3dData() {
    Character3DData* newData = (Character3DData*) malloc (sizeof(Character3DData));
    if (!newData) {
        slog("Could not create Character3DData.");
        return NULL;
    }

    memset(newData, 0, sizeof(newData));

    newData->velocity = gfc_vector3d(0, 0, 0);
    newData->motionVelocity = gfc_vector3d(0, 0, 0);
    newData->gravityVelocity = gfc_vector3d(0, 0, 0);
    newData->gravity = BASE_GRAVITY;
    newData->rotation = gfc_vector3d(M_PI, 0, 0);
    newData->isOnFloor = false;
    newData->zSnap = 0;

    newData->horizontalCollisionRadius = BASE_HORIZONTAL_COLLISION_RADIUS;

    GFC_Edge3D gravityRaycast = { 0 };
    newData->gravityRaycast = gravityRaycast;

    return newData;
}

void moveAndSlide(Entity* self, Character3DData* character3dData, float delta) {
    bool wasOnFloor = character3dData->isOnFloor;
    character3dData->isOnFloor = false;

    GFC_Vector3D collisionPushback = { 0 };
    GFC_Vector3D nextMotionPosition = gfc_vector3d_added(self->position, gfc_vector3d(character3dData->motionVelocity.x * delta, character3dData->motionVelocity.y * delta, character3dData->motionVelocity.z * delta));
    GFC_Vector3D nextGravityPosition = gfc_vector3d_added(self->position, gfc_vector3d(character3dData->gravityVelocity.x * delta, character3dData->gravityVelocity.y * delta, character3dData->gravityVelocity.z * delta));
    GFC_Capsule baseCollisionCapsule = gfc_capsule(self->entityCollision->collisionPrimitive->s.c.height, self->entityCollision->collisionPrimitive->s.c.radius);
    setCapsuleFinalRadius(&baseCollisionCapsule, NULL);

    GFC_Capsule collisionCapsule = baseCollisionCapsule;
    collisionCapsule.finalBase = nextMotionPosition;
    setCapsuleFinalTip(&collisionCapsule, NULL);
    collide(self, character3dData, delta, &character3dData->isOnFloor, &collisionPushback, collisionCapsule, false);

    collide(self, character3dData, delta, &character3dData->isOnFloor, &collisionPushback, collisionCapsule, true);

    if (character3dData->isOnFloor || wasOnFloor) {
        character3dData->gravityVelocity.z = 0;
    }
    else {
        character3dData->gravityVelocity.z += BASE_GRAVITY * delta;
    }

    character3dData->velocity = gfc_vector3d_added(character3dData->motionVelocity, character3dData->gravityVelocity);


    self->position.x += character3dData->velocity.x * delta;
    self->position.y += character3dData->velocity.y * delta;
    self->position.z += character3dData->velocity.z * delta;
    
    self->position = gfc_vector3d_added(self->position, collisionPushback);
    

}

void collide(Entity* self, Character3DData* character3dData, float delta, bool* isOnFloor, GFC_Vector3D *collisionPushback, GFC_Capsule capsule, bool floorCollisions) {
    GFC_Vector3D velocity = character3dData->motionVelocity;

    GFC_Vector3D intersectionPoint = { 0 };
    GFC_Vector3D penetrationNormal = { 0 };
    float penetrationDepth = 0;

    PlayerData* playerData = NULL;
    bool collided = false;
    Entity* currEntity = NULL;
    for (int i = 0; i < entityManager.entityMax; i++) {
        currEntity = &entityManager.entityList[i];
        // Filter out inactive entities, non-collideable, and collideable out of range
        if (!currEntity->_in_use) {
            continue;
        }
        if (currEntity->entityCollision) {
            if (!gfc_box_overlap(self->entityCollision->AABB, currEntity->entityCollision->AABB)) {
                continue;
            }
        }
        if (!isOnLayer(currEntity, 3) && !isOnLayer(currEntity, 2) && !isOnLayer(currEntity, 1)) {
            continue;
        }
        if (currEntity == self) {
            continue;
        }

        if (entityCapsuleTest(currEntity, capsule, &intersectionPoint, &penetrationNormal, &penetrationDepth, &self->entityCollision->AABB)) {
            collided = true;
            if (!floorCollisions) {

                float velocityMagnitude = gfc_vector3d_magnitude(velocity);
                GFC_Vector3D normalizedVelocity = velocity;
                gfc_vector3d_normalize(&normalizedVelocity);

                float pushBack = (penetrationDepth + GFC_EPSILON);
                GFC_Vector3D pushbackVector = gfc_vector3d(pushBack, pushBack, pushBack);
                *collisionPushback = gfc_vector3d_added(
                    *collisionPushback,
                    gfc_vector3d_multiply(
                        penetrationNormal,
                        pushbackVector
                    )
                );

                //printf("\nPushback: %f", pushBack);
                //printVector3D("Pushback Vector", penetrationNormal);
            }
            else {
                *isOnFloor = gfc_vector3d_dot_product(penetrationNormal, gfc_vector3d(0, 0, 1));
            }
        }

    }

    if (!floorCollisions) {
        character3dData->motionVelocity = velocity;
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
        if (currEntity->entityCollision) {
            if (!gfc_box_overlap(self->entityCollision->AABB, currEntity->entityCollision->AABB)) {
                continue;
            }
        }
        if (!isOnLayer(currEntity, 3) && !isOnLayer(currEntity, 2)) {
            continue;
        }
        if (currEntity == self) {
            continue;
        }
        if (!gfc_vector3d_distance_between_less_than(entityGlobalPosition(self), entityGlobalPosition(currEntity), character3dData->horizontalCollisionRadius * 24)) {
            continue;
        }

        
        if (entityCapsuleTest(currEntity, self->entityCollision->collisionPrimitive->s.c, &intersectionPoint, &penetrationNormal, &penetrationDepth, NULL)) {
            GFC_Vector3D pushBack = penetrationNormal;

            pushBack.x *= penetrationDepth / delta;// *0.01;
            pushBack.y *= penetrationDepth / delta;// *0.01;
            pushBack.z *= penetrationDepth / delta;// *0.01;
            printVector3D("Pushback", pushBack);
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


        if (entityCapsuleTest(currEntity, self->entityCollision->collisionPrimitive->s.c, &intersectionPoint, &penetrationNormal, &penetrationDepth, &self->entityCollision->AABB)) {
            //printf("\nPenetraion Normal: %f, %f, %f", penetrationNormal.x, penetrationNormal.y, penetrationNormal.z);
            //printf("\nPenetration depth: %f", penetrationDepth);
            float velocityLength = gfc_vector3d_magnitude(velocity);
            GFC_Vector3D velocityNormalized = gfc_vector3d_multiply(velocity, gfc_vector3d(velocityLength / 1, velocityLength / 1, velocityLength / 1));
            float normalizedDot = gfc_vector3d_dot_product(velocityNormalized, penetrationNormal);
            GFC_Vector3D undesiredMotion = gfc_vector3d_multiply(penetrationNormal, gfc_vector3d(normalizedDot, normalizedDot, normalizedDot));
            GFC_Vector3D desiredMotion = gfc_vector3d_subbed(velocityNormalized, undesiredMotion);


            GFC_Vector3D horizontalDirection = gfc_vector3d(character3dData->velocity.x, character3dData->velocity.y, 0); 
            gfc_vector3d_normalize(&horizontalDirection);
            float dotProduct = gfc_vector3d_dot_product(penetrationNormal, horizontalDirection);
            float horizontalMagnitude = sqrt(character3dData->velocity.x * character3dData->velocity.x + character3dData->velocity.y * character3dData->velocity.y);
            if (dotProduct > 0) {
                velocity.z = -tanf(dotProduct) * horizontalMagnitude;
            }
            else if (dotProduct < 0) {
                velocity.z = 0;
            } else
            {
                velocity.z = 0;
            }
            float epsilonDepth = penetrationDepth + GFC_EPSILON * 0.1;
            GFC_Vector3D epsilonAdjustment = gfc_vector3d_multiply(penetrationNormal, gfc_vector3d(epsilonDepth, epsilonDepth, epsilonDepth));
            self->position = gfc_vector3d_added(self->position, epsilonAdjustment);
            
            //printf("\n%f", penetrationNormal.z);
            isOnFloor = true;

        }

    }


    if (!isOnFloor) {
        character3dData->zSnap = 0;
        velocity.z += character3dData->gravity;
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
                *floorNormal = gfc_trigfc_angle_get_normal(t);
                return true;
            }
        }

    }
    return false;
}