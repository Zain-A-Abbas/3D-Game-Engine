#include "Level.h"
#include "simple_logger.h"

LevelData *createForestLevel(Entity *player) {
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
    player = createPlayer();
    if (!player) {
        return NULL;
    }
    assignCamera(player, gf3dGetCamera());
    player->position.z = 0;

    // Setup enemy
    Entity* enemy1 = createZombie(player);
    enemy1->position = gfc_vector3d(0, -40, 0);
}

void *levelDraw(LevelData *levelData) {
    LevelLayout *layout = levelData->layout;
    gf3d_model_draw_sky(
        layout->sky,
        layout->skyMat,
        GFC_COLOR_WHITE
    );
}