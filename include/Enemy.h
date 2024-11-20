#ifndef __ENEMY__
#define __ENEMY__

#include "Entity.h"
#include "Character3D.h"
#include "StateMachine.h"


typedef struct {
    char    dyingAnimName[32];
} DyingData;

typedef struct EnemyData_S {
    StateMachine        *enemyStateMachine;
    Character3DData     *character3dData;
    float               aiTime; // The time used to gauge gaps between AI decisions
    int                 hp;
    GFC_Sphere          attackSphere;
} EnemyData;


/**
* @brief Creates an enemy entity and assigns it the proper collision layer.
*/
Entity * enemyEntityNew();

void enemyThink(Entity* self, float delta);
void enemyUpdate(Entity* self, float delta);

void enemyAttacked(Entity* self, int damage);

void giveDeathState(StateMachine* stateMachine, char dyingAnimation[32]);
void dyingEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void dyingThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void dyingUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);


void enemyDelete(Entity* self);

void enemySetCollision(Entity* self, float height, float radius);

#endif