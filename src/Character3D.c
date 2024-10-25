#include "Character3D.h"

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
    GFC_Edge3D gravityRaycast = gfc_edge3d_from_vectors(self->position, gfc_vector3d_added(self->position, gravityRaycastDir));
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