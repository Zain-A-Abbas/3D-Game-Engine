#include "Level.h"
#include "simple_logger.h"
#include "Player.h"
#include "TerrainManager.h"
#include "Powerup.h"
#include "Zombie.h"
#include "Arm.h"
#include "Interactable.h"
#include "ShopItem.h"
#include "Structure.h"
#include "light.h"
#include "Ghost.h"

const int FOREST_LEVEL_ENEMY_COUNT = 25;
const int BEACH_LEVEL_ENEMY_COUNT = 45;
const float FOREST_ENEMY_SPAWN_TIME = 12.5;
const float BEACH_ENEMY_SPAWN_TIME = 10.5;
const float POWERUP_SPAWN_TIME = 20.0;

LevelData *levelData = NULL;

LevelData *createForestLevel(Entity **player) {
    LevelLayout *layout = (LevelLayout*) malloc(sizeof(LevelLayout));
    if (!layout) {
        slog("Could not create level layout");
        return NULL;
    }
    memset(layout, 0, sizeof(LevelLayout));

    // Level Data
    LevelData *data = (LevelData*) malloc(sizeof(LevelData));
    if (!data) {
        slog("Could not create level data");
        free(layout);
        return NULL;
    }
    memset(data, 0, sizeof(LevelData));
    data->layout = layout;
    data->enemyTargetCount = FOREST_LEVEL_ENEMY_COUNT;
    data->enemySpawnTimer = -5;
    data->powerupSpawnTimer = -15;
    data->spawnLowerBound = gfc_vector3d(-350, -350, -4);
    data->spawnUpperBound = gfc_vector3d(350, 350, -4);
    data->enemySpawnTime = FOREST_ENEMY_SPAWN_TIME;
    if (data->spawnLowerBound.x > data->spawnUpperBound.x || data->spawnLowerBound.y > data->spawnUpperBound.y || data->spawnLowerBound.z > data->spawnUpperBound.z) {
        slog("Incorrect relationship between spawn bounds. Rejecting creating level");
    }

    // Setup skybox
    Model *daySky, *nightSky;
    daySky = gf3d_model_load("models/daysky.model");
    nightSky = gf3d_model_load("models/nightsky.model");
    gfc_matrix4_identity(layout->skyMat);
    layout->daySky = daySky;
    layout->nightSky = nightSky;

    // Setup ground
    Entity* testGround = terrainEntityNew();
    //testGround->model = gf3d_model_load("models/primitives/testground.model");
    testGround->model = gf3d_model_load("models/structures/Ground1.model");
    //testGround->scale = gfc_vector3d(4, 4, 1);
    testGround->position = gfc_vector3d(0, 0, -8);
    EntityCollision* groundCollision = (EntityCollision*)malloc(sizeof(EntityCollision));
    memset(groundCollision, 0, sizeof(EntityCollision));
    testGround->entityCollision = groundCollision;
    GFC_Box testGroundbox = gfc_box(-400, -400, -40, 800, 800, 80);
    newQuadTree(testGround, testGroundbox, 5);

    // Setup lights
    addDirectionalLight(
        gfc_color_to_vector4(gfc_color(0.02, 0.0, 0.1, 1.0)),
        gfc_vector4d(0.707, 0, -0.707, 0.0),
        0.1
    );
    addDirectionalLight(
        gfc_color_to_vector4(gfc_color(0.08, 0.0, 0.01, 1.0)),
        gfc_vector4d(-0.707, 0, 0.707, 0.0),
        0.05
    );

    // Setup player
    Entity* playerEntity = createPlayer();
    if (!playerEntity) {
        return NULL;
    }
    assignCamera(playerEntity, gf3dGetCamera());
    playerEntity->position.z = 0;
    *player = playerEntity;
    data->player = playerEntity;

    // Create Level border
    createForestBorder(gfc_vector3d(-200, 380, -16), 0);
    createForestBorder(gfc_vector3d(200, 380, -16), 0);
    createForestBorder(gfc_vector3d(-200, -380, -16), 0);
    createForestBorder(gfc_vector3d(200, -380, -16), 0);
    createForestBorder(gfc_vector3d(-380, 200, -16), GFC_HALF_PI);
    createForestBorder(gfc_vector3d(380, 200, -16), GFC_HALF_PI);
    createForestBorder(gfc_vector3d(-380, -200, -16), GFC_HALF_PI);
    createForestBorder(gfc_vector3d(380, -200, -16), GFC_HALF_PI);

    // Create trees
    TerrainData*treeData;
    int treeCount = 160 + gfc_random_int(40);
    for (int i = 0; i < treeCount; i++) {
        float treeX = -375 + gfc_random_int(750);
        float treeY = -375 + gfc_random_int(750);
        float treeZ = -20 + gfc_random_int(4);
        Entity* testTree = terrainEntityNew();
        testTree->model = gf3d_model_load("models/structures/Tree.model");
        testTree->position = gfc_vector3d(treeX, treeY, treeZ);
        testTree->rotation.z = gfc_random() * GFC_2PI;
        testTree->scale.z = 0.8 + 0.4 * gfc_random();
        float xyScale = 0.9 + 0.2 * gfc_random();
        testTree->scale.x = xyScale;
        testTree->scale.y = xyScale;
        testTree->collisionLayer = 0b00000100;
        testTree->type = TERRAIN;
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


    /*Shop debugging*/
    Entity* shop = structureNew(STORE);
    shop->position = gfc_vector3d(0, 0, 1000);
    Entity* shopEntryDoor = createShopEntryDoor();
    Entity* shopExitDoor = createShopExitDoor(shop);

    Entity* shopMagnum = shopItemNew(MAGNUM);
    shopMagnum->position = gfc_vector3d(36, -48, 4);
    Entity* shopAutoShotgun = shopItemNew(AUTO_SHOTGUN);
    shopAutoShotgun->position = gfc_vector3d(24, -48, 4);
    Entity* shopAssaultRifle = shopItemNew(ASSAULT_RIFLE);
    shopAssaultRifle->position = gfc_vector3d(12, -48, 4);
    Entity* shopMinigun = shopItemNew(MINIGUN);
    shopMinigun->position = gfc_vector3d(-12, -48, 4);
    Entity* shopRocketLauncher = shopItemNew(ROCKET_LAUNCHER);
    shopRocketLauncher->position = gfc_vector3d(-24, -48, 4);
    Entity* shopCrossbow = shopItemNew(CROSSBOW);
    shopCrossbow->position = gfc_vector3d(-36, -48, 4);

    shopMagnum->parent = shop;
    shopAutoShotgun->parent = shop;
    shopAssaultRifle->parent = shop;
    shopMinigun->parent = shop;
    shopRocketLauncher->parent = shop;
    shopCrossbow->parent = shop;

    levelData = data;

    return data;
}


LevelData* createBeachLevel(Entity** player, LevelData *forestLevelData) {
    LevelLayout* layout = (LevelLayout*)malloc(sizeof(LevelLayout));
    if (!layout) {
        slog("Could not create level layout");
        return NULL;
    }
    memset(layout, 0, sizeof(LevelLayout));

    // Level Data
    LevelData* data = (LevelData*)malloc(sizeof(LevelData));
    if (!data) {
        slog("Could not create level data");
        free(layout);
        return NULL;
    }
    memset(data, 0, sizeof(LevelData));
    data->layout = layout;
    data->enemyTargetCount = BEACH_LEVEL_ENEMY_COUNT;
    data->enemySpawnTimer = -5;
    data->powerupSpawnTimer = -15;
    data->spawnLowerBound = gfc_vector3d(-320, -300, -4);
    data->spawnUpperBound = gfc_vector3d(320, 300, -4);
    data->enemySpawnTime = BEACH_ENEMY_SPAWN_TIME;
    if (data->spawnLowerBound.x > data->spawnUpperBound.x || data->spawnLowerBound.y > data->spawnUpperBound.y || data->spawnLowerBound.z > data->spawnUpperBound.z) {
        slog("Incorrect relationship between spawn bounds. Rejecting creating level");
    }

    // Setup skybox
    Model* daySky, * nightSky;
    daySky = forestLevelData->layout->daySky;
    nightSky = forestLevelData->layout->nightSky;
    gfc_matrix4_identity(layout->skyMat);
    layout->daySky = daySky;
    layout->nightSky = nightSky;

    // Setup ground
    Entity* testGround = terrainEntityNew();
    //testGround->model = gf3d_model_load("models/primitives/testground.model");
    testGround->model = gf3d_model_load("models/structures/BeachGround.model");
    testGround->position = gfc_vector3d(0, 0, -8);
    EntityCollision* groundCollision = (EntityCollision*)malloc(sizeof(EntityCollision));
    memset(groundCollision, 0, sizeof(EntityCollision));
    testGround->entityCollision = groundCollision;
    GFC_Box testGroundbox = gfc_box(-400, -400, -40, 800, 800, 80);
    newQuadTree(testGround, testGroundbox, 5);

    Entity* water = entityNew();
    water->model = gf3d_model_load("models/structures/Water.model");
    water->position = gfc_vector3d(0, 0, -8);
    Entity* water2 = entityNew();
    water2->model = gf3d_model_load("models/structures/Water.model");
    water2->position = gfc_vector3d(-1200, -1200, -8);
    Entity* water3 = entityNew();
    water3->model = gf3d_model_load("models/structures/Water.model");
    water3->position = gfc_vector3d(0, -1200, -8);
    Entity* water4 = entityNew();
    water4->model = gf3d_model_load("models/structures/Water.model");
    water4->position = gfc_vector3d(1200, -1200, -8);
    Entity* water5 = entityNew();
    water5->model = gf3d_model_load("models/structures/Water.model");
    water5->position = gfc_vector3d(-1200, 0, -8);
    Entity* water6 = entityNew();
    water6->model = gf3d_model_load("models/structures/Water.model");
    water6->position = gfc_vector3d(1200, 0, -8);
    Entity* water7 = entityNew();
    water7->model = gf3d_model_load("models/structures/Water.model");
    water7->position = gfc_vector3d(-1200, 1200, -8);
    Entity* water8 = entityNew();
    water8->model = gf3d_model_load("models/structures/Water.model");
    water8->position = gfc_vector3d(0, 1200, -8);
    Entity* water9 = entityNew();
    water9->model = gf3d_model_load("models/structures/Water.model");
    water9->position = gfc_vector3d(1200, 1200, -8);

    // Setup player
    data->player = forestLevelData->player;
    data->player->position = gfc_vector3d(0, 0, 0);
    PlayerData* playerData = getPlayerData(data->player);
    playerData->state = PS_FREE;

    // Create Level border
    Entity* beachBorder = entityNew();
    beachBorder->collisionLayer = 0b00000010;
    beachBorder->model = gf3d_model_load("models/structures/BeachBorderCollision.model");

    // Create trees
    TerrainData* treeData;
    int treeCount = 160 + gfc_random_int(40);
    for (int i = 0; i < treeCount; i++) {
        float treeX = -300 + gfc_random_int(600);
        float treeY = -300 + gfc_random_int(600);
        float treeZ = -40 + gfc_random_int(15);
        Entity* testTree = terrainEntityNew();
        testTree->model = gf3d_model_load("models/structures/PalmTree.model");
        testTree->position = gfc_vector3d(treeX, treeY, treeZ);
        testTree->rotation.z = gfc_random() * GFC_2PI;
        testTree->scale.z = 0.8 + 0.4 * gfc_random();
        float xyScale = 0.9 + 0.2 * gfc_random();
        testTree->scale.x = xyScale;
        testTree->scale.y = xyScale;
        testTree->collisionLayer = 0b00000100;
        testTree->type = TERRAIN;
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

    levelData = data;
    free(forestLevelData);

    return data;
}

void *levelDraw() {
    if (!levelData) {
        return;
    }
    if (gfc_input_command_pressed("debugTimePass")) {
        levelData->dayNightSeconds += 10;
    }

    LevelLayout *layout = levelData->layout;

    int skyFileNumber = 1;
    if (levelData->dayNightSeconds >= 120 && levelData->dayNightSeconds < 180) {
        skyFileNumber = 2 + (levelData->dayNightSeconds - 120) / 10;
    }
    else if (levelData->dayNightSeconds >= 180 && levelData->dayNightSeconds < 300) {
        skyFileNumber = 8;
    }
    else if (levelData->dayNightSeconds >= 300 && levelData->dayNightSeconds < 360) {
        skyFileNumber = 8 - 1 - (levelData->dayNightSeconds - 300) / 10;
    }

    char skyFile[20];
    sprintf(skyFile, "models/sky/sky%d.png", skyFileNumber);
    Texture* skyTexture = gf3d_texture_load(skyFile);
    layout->daySky->texture = skyTexture;
    gf3d_model_draw_sky(
        layout->daySky,
        layout->skyMat,
        GFC_COLOR_WHITE
    );
}

void createForestBorder(GFC_Vector3D position, float zRotation) {
    Entity* border = entityNew();
    border->type = TERRAIN;
    border->collisionLayer = 0b00000010;
    border->model = gf3d_model_load("models/structures/tree_border.model");
    border->position = position;
    border->rotation.z = zRotation;
}

void levelEnemyKilled() {
    levelData->enemyKillCount += 1;
    levelData->aliveEnemies -= 1;
    printf("\nEnemies killed: %d", levelData->enemyKillCount);

    // If all enemies killed...
    if (levelData->enemyKillCount >= levelData->enemyTargetCount) {
        // Disable player input
        PlayerData* playerData = getPlayerData(levelData->player);
        playerData->state = PS_NONE;
        
        // Erase all enemies
        for (int i = 0; i < entityManager.entityMax; ++i) {
            if (entityManager.entityList[i]._in_use) continue;
            if (entityManager.entityList[i].type == ENEMY) {
                enemyDelete(&entityManager.entityList[i]);
            }
        }

        levelData->levelClear = true;
        levelData->levelClearTimer = 2.0;
    }
}

void levelProcess(float delta) {
    if (!levelData) {
        return;
    }

    if (levelData->levelClear) {
        levelData->levelClearTimer -= delta;
        if (levelData->levelClearTimer <= 0.0) {
            levelData->levelClear = false;
            clearForestLevel();
            createBeachLevel(&levelData->player, levelData);
        }
        return;
    }

    levelData->enemySpawnTimer += delta;
    levelData->powerupSpawnTimer += delta;
    levelData->dayNightTracker += delta;

    if (levelData->enemySpawnTimer > ( (levelData->enemySpawnTime) - levelData->enemySpawnTime * 0.33 * (levelData->dayNightSeconds > 180) ) ) {
        int spawnCount = gfc_random_int(3) + 1;
        for (int i = 0; i < spawnCount; ++i) {
            spawnEnemy();
        }
        levelData->enemySpawnTimer = 0.0;
    }

    if (levelData->powerupSpawnTimer > POWERUP_SPAWN_TIME) {
        spawnPowerup();
        levelData->powerupSpawnTimer = 0.0;
    }

    if (levelData->dayNightTracker >= 1.0) {
        levelData->dayNightSeconds += 1;
        levelData->dayNightTracker = 0.0;
        if (levelData->dayNightSeconds >= 360) {
            levelData->dayNightSeconds = 0;
        }
        //printf("\nSeconds: %d", levelData->dayNightSeconds);
    }

    if (gfc_input_command_pressed("debugKillCount")) {
        levelEnemyKilled();
    }
}

void spawnEnemy() {
    if (levelData->aliveEnemies >= 15) {
        return;
    }
    int enemyType = gfc_random_int(4); // Based on EnemyType enum
    GFC_Vector3D enemySpawnPosition = gfc_vector3d_added(
        levelData->spawnLowerBound,
        gfc_vector3d(
            gfc_random_int(levelData->spawnUpperBound.x - levelData->spawnLowerBound.x),
            gfc_random_int(levelData->spawnUpperBound.y - levelData->spawnLowerBound.y),
            gfc_random_int(levelData->spawnUpperBound.z - levelData->spawnLowerBound.z)
        )
    );
    Entity* newEnemy = NULL;
    switch (enemyType)
    {
    case 0:
    case 1:
        newEnemy = createZombie(levelData->player);
        break;
    case 2:
        newEnemy = createArm(levelData->player);
        break;
    case 3:
        newEnemy = createGhost(levelData->player);
        newEnemy->position.z += gfc_random_int(20) + 20;
        break;
    default:
        break;
    }

    newEnemy->position = gfc_vector3d_added(newEnemy->position, enemySpawnPosition);
    levelData->aliveEnemies += 1;
}

void clearForestLevel() {
    for (int i = 0; i < entityManager.entityMax; ++i) {
        if (!entityManager.entityList[i]._in_use) continue;
        Entity* currEntity = &entityManager.entityList[i];
        Bool delete = true;
        if (currEntity->type == PLAYER || currEntity->type == STRUCTURE || currEntity->type == INTERACTABLE) {
            delete = false;
        }

        if (delete) {
            _entityFree(currEntity);
  
        }
    }
}

void spawnPowerup() {
    GFC_Vector3D powerupSpawnPosition = gfc_vector3d_added(
        levelData->spawnLowerBound,
        gfc_vector3d(
            gfc_random_int(levelData->spawnUpperBound.x - levelData->spawnLowerBound.x),
            gfc_random_int(levelData->spawnUpperBound.y - levelData->spawnLowerBound.y),
            gfc_random_int(levelData->spawnUpperBound.z - levelData->spawnLowerBound.z)
        )
    );

    Entity* newPowerup = powerupEntityNew(levelData->player);
    newPowerup->model = gf3d_model_load("models/dino.model");
    newPowerup->position = powerupSpawnPosition;
}