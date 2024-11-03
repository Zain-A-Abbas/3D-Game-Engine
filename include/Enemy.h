#ifndef __ENEMY__
#define __ENEMY__

#include "Entity.h"
#include "Character3D.h"
#include "StateMachine.h"

typedef struct EnemyData_S {
    StateMachine        *enemyStateMachine;
    Character3DData     *character3dData;
    float               aiTime; // The time used to gauge gaps between AI decisions
    Model               *enemyCollision;
    int                 hp;
} EnemyData;


/**
* @brief Creates an enemy entity and assigns it the proper collision layer.
*/
Entity * enemyEntityNew();

void enemyThink(Entity* self, float delta);
void enemyUpdate(Entity* self, float delta);

void enemyAttacked(Entity* self, int damage);
void enemyDelete(Entity* self);

void enemyScalePreserveModel(Entity* self, GFC_Vector3D scale);

#endif