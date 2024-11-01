#include "Character3D.h"
#include "simple_logger.h"

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
    GFC_Vector3D raycastStart = gfc_vector3d(0, 0, 0);
    GFC_Vector3D raycastEnd = gfc_vector3d(0, 0, 0);
    GFC_Edge3D movementRaycast = gfc_edge3d_from_vectors(raycastStart, raycastEnd);
    GFC_Vector3D contact = gfc_vector3d(0, 0, 0);
    GFC_Triangle3D t = { 0 };
    GFC_Vector3D normalizedVelocity = gfc_vector3d(0, 0, 0);
    GFC_Vector3D triangleNormal = gfc_vector3d(0, 0, 0);
    GFC_Vector3D velocitySubtract = gfc_vector3d(0, 0, 0);
    float angle = 0;
    float dot = 0;
    float speedMod = 0;
    float velocityMagnitude = 0;

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Filter out inactive entities, non-collideable, and collideable out of range
        Entity* currEntity = &entityManager.entityList[i];
        if (!currEntity->_in_use) {
            continue;
        }
        if (!isOnLayer(currEntity, 3)) {
            continue;
        }
        if (!gfc_vector3d_distance_between_less_than(entityGlobalPosition(self), entityGlobalPosition(currEntity), character3dData->horizontalCollisionRadius * 24)) {
            continue;
        }
        
        // Constructs 16 raycasts in a circular perimeter around the entity
        for (int j = 0; j < 16; j++) {
            angle = M_PI / 8 * j;
            raycastStart = gfc_vector3d(0, character3dData->horizontalCollisionRadius, 0);
            gfc_vector3d_rotate_about_z(&raycastStart, angle);
            raycastStart = gfc_vector3d_added(raycastStart, entityGlobalPosition(self));
            raycastEnd = gfc_vector3d_added(raycastStart, velocity);
            movementRaycast = gfc_edge3d_from_vectors(raycastStart, raycastEnd);
            normalizedVelocity = velocity;
            velocityMagnitude = gfc_vector3d_magnitude(velocity);

            // Check each raycast in the direction of movement
            //slog("Velocity start: %f, %f, %f", raycastStart.x, raycastStart.y, raycastStart.z);
            if (entityRaycastTest(currEntity, movementRaycast, &contact, &t, NULL)) {
                // Take the dot product between the direction of horizontal velocity and the normal of the wall
                gfc_vector3d_normalize(&normalizedVelocity);
                triangleNormal = gfc_trigfc_angle_get_normal(t);
                dot = gfc_vector3d_dot_product(normalizedVelocity, triangleNormal);

                speedMod = 1.0 - fabsf(dot * dot);
                // Create velocitySubtract by multiplying the normal of the triangle by the dot product and the speed at which the entity was traveling
                velocitySubtract = gfc_vector3d_multiply(triangleNormal, gfc_vector3d(dot * velocityMagnitude, dot * velocityMagnitude, dot * velocityMagnitude));

                velocity = gfc_vector3d_subbed(velocity, velocitySubtract);
                // Use the dot product to slow down if hitting wall at an angle rather than turning with no speed loss
                velocity = gfc_vector3d_multiply(velocity, gfc_vector3d(speedMod, speedMod, speedMod));

            }

        }

    }
    character3dData->velocity = velocity;
}

GFC_Vector3D verticalVectorMovement(Entity * self, Character3DData * character3dData, float delta) {
    
    GFC_Vector3D floorNormal;
    GFC_Vector3D contact = { 0 };
    if (isOnFloor(self, character3dData, &floorNormal, &contact)) {
        float floorAngle = M_PI / 2 - asinf(floorNormal.z);
        // Make it snap to a certain height when traversing on ground with a lower angle than on the previous frame to account for slightly dipping into the floor
        if (character3dData->previousFloorAngle > floorAngle) {
            character3dData->zSnapTarget = contact.z - character3dData->gravityRaycastHeight;
            character3dData->zSnap;
        }

        character3dData->previousFloorAngle = floorAngle;

        // Get the normalized non-vertical movement
        GFC_Vector3D horizontalDirection = gfc_vector3d(character3dData->velocity.x, character3dData->velocity.y, 0);
        gfc_vector3d_normalize(&horizontalDirection);
        // Used to determine if moving up or down
        float dotProduct = gfc_vector3d_dot_product(floorNormal, horizontalDirection);
        float horizontalMagnitude = sqrtf(pow(character3dData->velocity.x, 2) + pow(character3dData->velocity.y, 2));

        // Get the dot product as if parallel to the slope, to slow down vertical movement in case the player is not moving parallel to it
        GFC_Vector3D opposite = gfc_vector3d(-floorNormal.x, -floorNormal.y, 0);
        gfc_vector3d_normalize(&opposite);
        float parallelDotProduct = gfc_vector3d_dot_product(floorNormal, opposite);

        // Uses tanf rather than sinf as the horizontal speed should not adjust with the angle of the slope
        float floorRatio = tanf(floorAngle) * fabs(dotProduct / parallelDotProduct);

        if (dotProduct < 0) {
            character3dData->velocity.z = horizontalMagnitude * floorRatio;
        }
        else if (dotProduct > 0) {
            character3dData->velocity.z = -horizontalMagnitude * floorRatio;
        }
        else {
            character3dData->velocity.z = 0;
        }

    }
    else {
        character3dData->zSnap = 0;
        character3dData->velocity.z += character3dData->gravity * delta;
    }

}

int isOnFloor(Entity* self, Character3DData* character3dData, GFC_Vector3D* floorNormal, GFC_Vector3D* contact) {
    GFC_Triangle3D t = { 0 };
    GFC_Vector3D gravityRaycastDir = gfc_vector3d(0, 0, character3dData->gravityRaycastHeight);
    GFC_Edge3D gravityRaycast = gfc_edge3d_from_vectors(gfc_vector3d_added(entityGlobalPosition(self), gravityRaycastDir), entityGlobalPosition(self));
    
    for (int i = 0; i < entityManager.entityMax; i++) {
        // Get ground entitiesentities
        if (!entityManager.entityList[i]._in_use) {
            continue;
        }
        if (!isOnLayer(&entityManager.entityList[i], 1)) {
            continue;
        }

        // Found terrain
        if (entityRaycastTest(&entityManager.entityList[i], gravityRaycast, contact, &t, NULL)) {
            /*slog("%f, %f, %f", t.a.x, t.a.y, t.a.z);
            slog("%f, %f, %f", t.b.x, t.b.y, t.b.z);
            slog("%f, %f, %f", t.c.x, t.c.y, t.c.z);*/
            *floorNormal = gfc_trigfc_angle_get_normal(t);
            return true;
        }
    }
    return false;
}