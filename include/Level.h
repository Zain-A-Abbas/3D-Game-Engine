#ifndef __LEVEL__
#define __LEVEL__

#include "gf3d_model.h"
#include "Entity.h"

typedef enum {
    FOREST,
    BEACH
} Levels;

typedef struct {
    Model           *daySky;
    Model           *nightSky;
    GFC_Matrix4     skyMat;
    Entity          *terrain;
    GFC_List        *generatedTerrain;
} LevelLayout;

typedef struct {
    LevelLayout         *layout;
    GFC_List            *enemies;
    Entity              *player;
    int                 enemyTargetCount;
    int                 enemyKillCount;
    int                 aliveEnemies;
    int                 time;
    int                 dayNight;
    float               enemySpawnTimer;
    float               enemySpawnTime;
    float               powerupSpawnTimer;
    GFC_Vector3D        spawnLowerBound; // Lower xyz values for where things are able to spawn. Every value in this must not be higher than the corresponding axis in spawnUpperBound
    GFC_Vector3D        spawnUpperBound;

    float               dayNightTracker;
    int                 dayNightSeconds;

    Bool                levelClear;
    float               levelClearTimer; // How long the game should stay frozen before moving on to the next thing
} LevelData;


extern LevelData *levelData;

LevelData *createForestLevel(Entity **player);

void *levelDraw();
void levelEnemyKilled();
void levelProcess(float delta);
void spawnEnemy();
void spawnPowerup();
void createForestBorder(GFC_Vector3D position, float zRotation);
void clearForestLevel();

#endif