#ifndef __ZOMBIE__
#define __ZOMBIE__

#include "Enemy.h"
#include "StateMachine.h"

typedef struct {
	Entity			*player;
} WanderData;

typedef struct {
	Entity			*player;
} ChaseData;

Entity * createZombie(Entity *player);

void chaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void chaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);

#endif