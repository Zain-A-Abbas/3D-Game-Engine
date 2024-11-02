#include "simple_logger.h"
#include "gfc_matrix.h"
#include "gf3d_obj_load.h"
#include "Entity.h"
#include "Enemy.h"



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
    entityDebugDraw(self, matrix);
}

void entityDebugDraw(Entity* self, GFC_Matrix4 matrix) {

    if (self->type == ENEMY) {
        EnemyData* enemyData = (EnemyData*)self->data;
        if (enemyData->enemyCollision) {
            gf3d_model_draw(
                enemyData->enemyCollision,
                matrix,
                GFC_COLOR_WHITE,
                NULL,
                0
            );
        }
    }
}

void _entityThink(Entity * self, float delta) {
    if (!self) return;
    
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
    gf3d_model_free(self->model);
    free(self->entityAnimation);
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


int entityRaycastTest(Entity * entity, GFC_Edge3D raycast, GFC_Vector3D *contact, GFC_Triangle3D * t, GFC_Box *boundingBox) {
    if (boundingBox) {
        GFC_Box localBox = { boundingBox->x, boundingBox->y, boundingBox->z, boundingBox->w, boundingBox->d, boundingBox->h };
        if (!gfc_point_in_box(entityGlobalPosition(entity), localBox)) {
            return false;
        }
    }

    Model* entityModel = entity->model;
    if (entity->type == ENEMY) {
        EnemyData* enemyData = (EnemyData*)entity->data;
        if (enemyData->enemyCollision) {
            entityModel = enemyData->enemyCollision;
        }
    }
    // Get meshes
    for (int j = 0; j < gfc_list_get_count(entityModel->mesh_list); j++) {
        Mesh* mesh = (Mesh*)gfc_list_get_nth(entityModel->mesh_list, j);
        if (mesh) {
            // Get primitives
            for (int k = 0; k < gfc_list_get_count(mesh->primitives); k++) {
                MeshPrimitive* primitive = (MeshPrimitive*)gfc_list_get_nth(mesh->primitives, k);
                if (primitive) {
                    if (primitive->objData) {
                        if (gf3d_entity_obj_line_test(primitive->objData, entity, raycast, contact, t)) {
                            return true;
                        }
                    }
                }
            }

        }
    }
    return 0;
}


void animationSetup(Entity* self, const char* animFolder) {
    if (self->entityAnimation) {
        free(self->entityAnimation);
    }
    self->entityAnimation = (EntityAnimation*)malloc(sizeof(EntityAnimation));
    if (!self->entityAnimation) {
        slog("Could not create entity animation handler");
        return;
    }

    memset(self->entityAnimation, 0, sizeof(EntityAnimation));
    self->entityAnimation->animFolder = animFolder;
}


void animationPlay(Entity* self, const char* animName) {
    if (!self->entityAnimation) {
        slog("Entity animation handler does not exist");
        return;
    }
    if (!self->entityAnimation->animFolder) {
        slog("Animlocation not defined");
        return;
    }

    const char* animation = malloc(strlen(self->entityAnimation->animFolder) + strlen(animName) + strlen(".model"));
    strcpy(animation, self->entityAnimation->animFolder);
    strcat(animation, animName);
    strcat(animation, ".model");

    gf3d_model_free(self->model);

    Model *newModel = gf3d_model_load(animation);
    if (!newModel) {
        slog("newModel animation could not be found");
        return;
    }
    self->model = newModel;

    if (!self->model->armature) {
        printf("\No armature");
        SJson* json, * config, * array;
        json = sj_load(animation);
        config = sj_object_get_value(json, "model");
        if (sj_object_get_value(config, "obj_list"))
        {
            array = sj_object_get_value(config, "obj_list");
            if (!array)
            {
                self->entityAnimation->animationFrameCount = 0;
            }
            else {
                self->entityAnimation->animationFrameCount = max(0, sj_array_get_count(array) - 1);
            }
        }

    }
    else {
        self->entityAnimation->animationFrameCount = max(0, self->model->armature->maxFrames);
    }
    self->entityAnimation->animationFrame = 0;
}

void entityAttacked(Entity* self, int damage) {
    if (self->type == ENEMY) {
        enemyAttacked(self, damage);
    }
}