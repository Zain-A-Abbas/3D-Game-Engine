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

typedef struct {
	Entity			*player;
	Bool			attacking;
} AttackData;

Entity * createZombie(Entity *player);

void wanderEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void wanderThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void wanderUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void wanderOnHit(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);

void chaseEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void chaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void chaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);

void attackEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void attackThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void attackUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);


#endif