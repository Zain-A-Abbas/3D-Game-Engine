#include "simple_logger.h"
#include "gfc_matrix.h"
#include "gf3d_obj_load.h"
#include "Entity.h"
#include "Enemy.h"
#include "Structure.h"
#include "TerrainManager.h"
#include "Projectile.h"
#include "gf3d_draw.h"

EntityManager entityManager = { 0 };

Entity * entityNew() {
    for (int i = 0; i < entityManager.entityMax; ++i) {
        if (entityManager.entityList[i]._in_use) continue; //Skil ones in use
        memset(&entityManager.entityList[i], 0, sizeof(Entity)); // Clear out memory if something is there
        entityManager.entityList[i].scale = gfc_vector3d(1, 1, 1);
        entityManager.entityList[i]._in_use = true;
        return &entityManager.entityList[i];
    }
    return NULL;
}

void entityDelete(Entity* self) {

}

void entitySystemInit(Uint32 maxEnts) {
    if (entityManager.entityList) {
        slog("Entity Manager exists.");
        return;
    }
    if (maxEnts == 0) {
        slog("Cannot allocate 0 entities.");
        return;
    }

    entityManager.entityList = (Entity*)gfc_allocate_array(sizeof(Entity), maxEnts);
    if (!entityManager.entityList) {
        slog("Failed to allocate %i entities for entity manager", maxEnts);
        return;
    }
    entityManager.entityMax = maxEnts;
    atexit(entitySystemClose);
}

void entitySystemClose() {
    for (int i = 0; i < entityManager.entityMax; ++i) {
        if (!entityManager.entityList[i]._in_use) continue;
        _entityFree(&entityManager.entityList[i]);
    }
    memset(&entityManager, 0, sizeof(EntityManager));
}


void _entityDraw(Entity * self) {
    if (!self) return;
    
    if (self->draw) {
        self->draw(self);
        return;
    }

    GFC_Matrix4 matrix;
    gfc_matrix4_from_vectors(
        matrix,
        entityGlobalPosition(self),
        entityGlobalRotation(self),
        entityGlobalScale(self)
    );

    /*Light light = {
        gfc_vector4d(0, 0, 0, 0),
        gfc_vector4d(0, 0.5, -1, 0),
        gfc_vector4d(1, 1, 1, 0),
        0.0,
        1.0,
        0.2,
        1.0,
        1.0
    };

    GFC_List *lightList = gfc_list_new();
    gfc_list_insert(lightList, &light, 0);

    int listCount = gfc_list_count(lightList);*/


    int animFrame = 0;
    if (self->entityAnimation) {
        animFrame = self->entityAnimation->animationFrame;
    }

    gf3d_model_draw(
        self->model,
        matrix,
        GFC_COLOR_WHITE,
        NULL,
        animFrame
    );
    //entityDebugDraw(self, matrix);
}

void entityDebugDraw(Entity* self, GFC_Matrix4 matrix) {

    /*if (self->entityCollision) {
        gf3d_model_draw(
            self->entityCollision,
            matrix,
            GFC_COLOR_WHITE,
            NULL,
            0
        );
    }*/



    /*if (self->type == PROJECTILE) {
        Projectile* data = (Projectile*)self->data;

            gf3d_draw_edge_3d(
                data->raycast,
                gfc_vector3d(0, 0, 0),
                gfc_vector3d(0, 0, 0),
                gfc_vector3d(1, 1, 1),
                1.0,
                GFC_COLOR_RED
            );
    }*/
}

void _entityThink(Entity * self, float delta) {
    if (!self) return;
    
    if (self->entityCollision) {
        GFC_Vector3D entityPosition = entityGlobalPosition(self);
        self->entityCollision->AABB.x = entityPosition.x - self->entityCollision->AABB.w / 2;
        self->entityCollision->AABB.y = entityPosition.y -self->entityCollision->AABB.d / 2;
        self->entityCollision->AABB.z = entityPosition.z -self->entityCollision->AABB.h / 2;

        if (self->entityCollision->collisionPrimitive->type == E_Capsule) {
            setCapsuleFinalBase(&self->entityCollision->collisionPrimitive->s.c, self);
            setCapsuleFinalTip(&self->entityCollision->collisionPrimitive->s.c, self);
        }
    }

    if (self->think) {
        self->think(self, delta);
        return;
    }
}


void _entityUpdate(Entity * self, float delta) {
    if (!self) return;

    if (self->entityAnimation) {
        if (self->entityAnimation->animationFrame+1 >= self->entityAnimation->animationFrameCount) {
            self->entityAnimation->animationFrame = 0;
        }
        else {
            self->entityAnimation->animationFrame += 1;
        }
    }
    
    if (self->update) {
        self->update(self, delta);
        return;
    }
}

void entityDrawAll() {
    for (int i = 0; i < entityManager.entityMax; ++i) {
        if (!entityManager.entityList[i]._in_use) continue;
        _entityDraw(&entityManager.entityList[i]);
    }
}

void entityUpdateAll(float delta) {
    if (delta > 1.0) {
        return;
    }
    for (int i = 0; i < entityManager.entityMax; ++i) {
        if (!entityManager.entityList[i]._in_use) continue;
        _entityUpdate(&entityManager.entityList[i], delta);
    }
}

void entityThinkAll(float delta) {
    if (delta > 1.0) {
        return;
    }
    for (int i = 0; i < entityManager.entityMax; ++i) {
        if (!entityManager.entityList[i]._in_use) continue;
        _entityThink(&entityManager.entityList[i], delta);
    }
}

void _entityFree(Entity *self) {
    if (!self) return;
    animationFree(self);
    gf3d_model_free(self->model);
    free(self->entityAnimation);
    if (self->entityCollision) {
        free(self->entityCollision);
    }
    memset(self, 0, sizeof(Entity));
}


int isOnLayer(Entity * self, int layer) {
    return ((self->collisionLayer >> layer) & 1);
}

GFC_Vector3D entityGlobalPosition(Entity* self) {
    GFC_Vector3D position = self->position;
    if (self->parent) {
        position = gfc_vector3d_added(position, entityGlobalPosition(self->parent));
    };
    return position;
}

GFC_Vector3D entityGlobalRotation(Entity* self) {
    GFC_Vector3D rotation = self->rotation;
    if (self->parent) {
        rotation = gfc_vector3d_added(rotation, entityGlobalRotation(self->parent));
    };
    return rotation;
}

GFC_Vector3D entityGlobalScale(Entity* self) {
    GFC_Vector3D scale = self->scale;
    if (self->parent) {
        scale = gfc_vector3d_multiply(scale, entityGlobalScale(self->parent));
    };
    return scale;
}

Uint8 entityCapsuleTest(Entity* entity, GFC_Capsule c, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth, GFC_Box *boundingBox) {
    if (boundingBox) {
        GFC_Box localBox = { boundingBox->x, boundingBox->y, boundingBox->z, boundingBox->w, boundingBox->d, boundingBox->h };
        if (!gfc_point_in_box(entityGlobalPosition(entity), localBox)) {
            return false;
        }
    }

    if (entity->entityCollision) {
        if (entity->entityCollision->collisionPrimitive->type == E_Capsule) {
            if (capsuleToCapsuleTest(c, entity->entityCollision->collisionPrimitive->s.c, intersectionPoint, penetrationNormal, penetrationDepth)) {
                return true;
            }
            return false;
        }
    }
    else {
        for (int j = 0; j < gfc_list_get_count(entity->model->mesh_list); j++) {
            Mesh* mesh = (Mesh*)gfc_list_get_nth(entity->model->mesh_list, j);
            if (mesh) {
                // Get primitives
                for (int k = 0; k < gfc_list_get_count(mesh->primitives); k++) {
                    MeshPrimitive* primitive = (MeshPrimitive*)gfc_list_get_nth(mesh->primitives, k);
                    if (primitive) {
                        if (primitive->objData) {
                            if (gf3d_entity_obj_capsule_test(primitive->objData, entity, c, intersectionPoint, penetrationNormal, penetrationDepth)) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;

}

int entityRaycastTest(Entity * entity, GFC_Edge3D raycast, GFC_Vector3D *contact, GFC_Triangle3D * t, GFC_Box *boundingBox) {
    if (boundingBox) {
        GFC_Box localBox = { boundingBox->x, boundingBox->y, boundingBox->z, boundingBox->w, boundingBox->d, boundingBox->h };
        if (!gfc_point_in_box(entityGlobalPosition(entity), localBox)) {
            return false;
        }
    }

    GFC_Vector3D* modelScale = NULL;

    Model* entityModel = entity->model;
    /*if (entity->entityCollision) {
        entityModel = entity->entityCollision;
        modelScale = (GFC_Vector3D*)malloc(sizeof(GFC_Vector3D));
        memset(modelScale, 0, sizeof(GFC_Vector3D));
        modelScale->x = entityModel->matrix[0][0];
        modelScale->y = entityModel->matrix[1][1];
        modelScale->z = entityModel->matrix[2][2];
    }*/


    // Get meshes
    for (int j = 0; j < gfc_list_get_count(entityModel->mesh_list); j++) {
        Mesh* mesh = (Mesh*)gfc_list_get_nth(entityModel->mesh_list, j);
        if (mesh) {
            // Get primitives
            for (int k = 0; k < gfc_list_get_count(mesh->primitives); k++) {
                MeshPrimitive* primitive = (MeshPrimitive*)gfc_list_get_nth(mesh->primitives, k);
                if (primitive) {
                    if (primitive->objData) {
                        if (gf3d_entity_obj_line_test(primitive->objData, entity, raycast, contact, t, modelScale)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return 0;
}


void animationSetup(Entity* self, const char* animFolder, char *animations[], int animationCount) {
    animationFree(self);
    EntityAnimation* newEntityAnim = (EntityAnimation*)malloc(sizeof(EntityAnimation));
    if (!newEntityAnim) {
        slog("Could not create entity animation handler");
        return;
    }
    memset(newEntityAnim, 0, sizeof(EntityAnimation));

    newEntityAnim->animationList = gfc_list_new();


    newEntityAnim->animFolder = animFolder;
    strcpy(newEntityAnim->currentAnimName, "");
    char* animation;

    for (int i = 0; i < animationCount; i++) {
        animation = malloc(strlen(newEntityAnim->animFolder) + strlen(animations[i]) + strlen(".model"));
        strcpy(animation, newEntityAnim->animFolder);
        strcat(animation, animations[i]);
        strcat(animation, ".model");

        Model* newModel = gf3d_model_load(animation);
        if (!newModel) {
            slog("Animation could not be found");
            return;
        }

        gfc_list_append(newEntityAnim->animationList, newModel);
    }
    self->entityAnimation = newEntityAnim;
}

void animationFree(Entity* self) {
    if (self->entityAnimation) {
        if (self->entityAnimation->animationList) {
            for (int i = 0; i < gfc_list_get_count(self->entityAnimation->animationList); i++) {
                if (gfc_list_get_nth(self->entityAnimation->animationList, i)) {
                    gf3d_model_free((Model*)gfc_list_get_nth(self->entityAnimation->animationList, i));
                }
            }
            gfc_list_delete(self->entityAnimation->animationList);
        }
        if (self->entityAnimation->animFolder) {
            self->entityAnimation->animFolder = NULL;
        }
        if (self->entityAnimation) {
            free(self->entityAnimation);
            self->entityAnimation = NULL;
        }    
    }
}

void animationPlay(Entity* self, const char* animName) {
    if (!self->entityAnimation) {
        slog("Entity animation handler does not exist");
        return;
    }
    if (!self->entityAnimation->animationList) {
        slog("No animation list");
        return;
    }

    Model* modelCheck;
    int modelIndex = -1;

    //printf("\nList size: %d", gfc_list_get_count(self->entityAnimation->animationList));
    for (int i = 0; i < gfc_list_get_count(self->entityAnimation->animationList); i++) {
        modelCheck = (Model*)gfc_list_get_nth(self->entityAnimation->animationList, i);
        //printf("\nAnimation in list is: %s", modelCheck->filename);
        //printf("\nAnimation given is: %s", animName);
        if (strcmp(modelCheck->filename, animName) == 0) {
            modelIndex = i;
            break;
        }
    }


    //printf("\nModel index is: %d", modelIndex);
    self->model = gfc_list_get_nth(self->entityAnimation->animationList, modelIndex);

    if (!self->model->armature) {
        // TODO: Add support for non-armatures in this animation format
        if (self->model->mesh_as_frame) {
            printf("\nFrame count: %d", gfc_list_get_count(self->model->mesh_list));
            self->entityAnimation->animationFrameCount = MAX(0, gfc_list_get_count(self->model->mesh_list));
        }
        else {
            self->entityAnimation->animationFrameCount = 0;
        }
    }
    else {
        self->entityAnimation->animationFrameCount = MAX(0, self->model->armature->maxFrames);
    }
    self->entityAnimation->animationFrame = 0;

}

void entityAttacked(Entity* self, int damage) {
    if (self->type == ENEMY) {
        enemyAttacked(self, damage);
    }
}
