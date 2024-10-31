#ifndef __ENEMY__
#define __ENEMY__

#include "Entity.h"
#include "StateMachine.h"

typedef struct EnemyData_S {
    GFC_Vector3D    enemyVelocity;
    GFC_Vector3D    enemyRotation;
    StateMachine    *enemyStateMachine;
} EnemyData;


/**
* @brief Creates an enemy entity and assigns it the proper collision layer.
*/
Entity * enemyEntityNew();

void enemyThink(Entity* self, float delta);
void enemyUpdate(Entity* self, float delta);

#endif