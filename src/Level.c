#include "Level.h"
#include "simple_logger.h"
#include "Player.h"
#include "TerrainManager.h"
#include "Zombie.h"
#include "Arm.h"

LevelData *createForestLevel(Entity **player) {
    LevelLayout *layout = (LevelLayout*) malloc(sizeof(LevelLayout));
    if (!layout) {
        slog("Could not create level layout");
        return NULL;
    }
    memset(layout, 0, sizeof(LevelLayout));

    LevelData *data = (LevelData*) malloc(sizeof(LevelData));
    if (!data) {
        slog("Could not create level data");
        free(layout);
        return NULL;
    }
    memset(data, 0, sizeof(LevelData));
    data->layout = layout;

    // Setup skybox
    Model *sky;
    sky = gf3d_model_load("models/sky.model");
    gfc_matrix4_identity(layout->skyMat);
    layout->sky = sky;

    // Setup ground
    Entity* testGround = terrainEntityNew();
    //testGround->model = gf3d_model_load("models/primitives/testground.model");
    testGround->model = gf3d_model_load("models/structures/Ground1.model");
    //testGround->scale = gfc_vector3d(4, 4, 1);
    testGround->position = gfc_vector3d(0, 0, -8);
    EntityCollision* groundCollision = (EntityCollision*)malloc(sizeof(EntityCollision));
    memset(groundCollision, 0, sizeof(EntityCollision));
    testGround->entityCollision = groundCollision;
    GFC_Box testGroundbox = gfc_box(-375, -375, -20, 750, 750, 40);
    newQuadTree(testGround, testGroundbox, 4);

    // Setup player
    Entity* playerEntity = createPlayer();
    if (!playerEntity) {
        return NULL;
    }
    assignCamera(playerEntity, gf3dGetCamera());
    playerEntity->position.z = -4;
    *player = playerEntity;

    // Create trees
    TerrainData*treeData;
    int treeCount = 160 + gfc_random_int(40);
    for (int i = 0; i < treeCount; i++) {
        float treeX = -375 + gfc_random_int(750);
        float treeY = -375 + gfc_random_int(750);
        float treeZ = -16 + gfc_random_int(4);
        Entity* testTree = terrainEntityNew();
        testTree->model = gf3d_model_load("models/structures/Tree.model");
        testTree->position = gfc_vector3d(treeX, treeY, treeZ);
        testTree->rotation.z = gfc_random() * GFC_2PI;
        testTree->collisionLayer = 0b00000100;
        treeData = (TerrainData*)testTree->data;

        EntityCollision* treeCollision = (EntityCollision*)malloc(sizeof(EntityCollision));
        memset(treeCollision, 0, sizeof(EntityCollision));

        GFC_ExtendedPrimitive* primitive = (GFC_ExtendedPrimitive*)malloc(sizeof(GFC_ExtendedPrimitive));
        memset(primitive, 0, sizeof(GFC_ExtendedPrimitive));
        primitive->type = E_Capsule;
        GFC_Capsule treeCapsule = gfc_capsule(16, 2);
                primitive->s.c = treeCapsule;
        treeCollision->collisionPrimitive = primitive;

        GFC_Box boundingBox = { 0 };
        boundingBox.x = -2; boundingBox.y = -2; boundingBox.z = -2;
        boundingBox.w = 4; boundingBox.d = 4; boundingBox.h = 4;
        treeCollision->AABB = boundingBox;

        testTree->entityCollision = treeCollision;
        //printf("\nTree location: %f, %f, %f", treeX, treeY, treeZ);
    }

    // Cube collision test
    Entity *cubeEntity = entityNew();
    cubeEntity->model = gf3d_model_load("models/primitives/cube.model");
    cubeEntity->collisionLayer = 0b00000010;
    cubeEntity->position.z = 0;
    cubeEntity->position.y = -20;

    // Setup enemy
    //Entity* enemy1 = createArm(playerEntity);
    //enemy1->position = gfc_vector3d(0, -40, 0);

    return data;
}

void *levelDraw(LevelData *levelData) {
    LevelLayout *layout = levelData->layout;
    gf3d_model_draw_sky(
        layout->sky,
        layout->skyMat,
        GFC_COLOR_WHITE
    );
}