#ifndef __GHOST__
#define __GHOST__

#include "Enemy.h"
#include "StateMachine.h"


typedef struct {
	Entity*			player;
	float			attackCheckTime;
	float			alpha;
} GhostChaseData;

typedef struct {
	Entity*			player;
	Bool			attacking;
	float			attackStartupTime;
} GhostAttackData;

Entity* createGhost(Entity* player);

void ghostChaseEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void ghostChaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void ghostChaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);

void ghostAttackEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void ghostAttackThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void ghostAttackUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);

void* ghostDraw(Entity* self, LightUBO* lights);

#endif