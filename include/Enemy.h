#ifndef __ENEMY__
#define __ENEMY__

#include "Entity.h"
#include "Character3D.h"
#include "StateMachine.h"


typedef struct {
    char        dyingAnimName[32];
    Entity      *player;
} DyingData;

typedef struct EnemyData_S {
    StateMachine        *enemyStateMachine;
    Character3DData     *character3dData;
    Entity              *player;
    int                 hp;
    GFC_Sphere          attackSphere;
} EnemyData;


/**
* @brief Creates an enemy entity and assigns it the proper collision layer.
*/
Entity * enemyEntityNew(Entity* player);

void enemyThink(Entity* self, float delta);
void enemyUpdate(Entity* self, float delta);

void enemyAttacked(Entity* self, int damage);

/**
* @brief Sets up a universal dying state for a given enemy
* @param stateMachine: The enemy's state machine.
* @param dyingAnimation: The animation to play when the enemy is killed.
*/
void giveDeathState(StateMachine* stateMachine, char dyingAnimation[32], Entity *player);
void dyingEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void dyingThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void dyingUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);


void enemyDelete(Entity* self);

void enemySetCollision(Entity* self, float height, float radius);

#endif