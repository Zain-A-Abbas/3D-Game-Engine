#ifndef __ARM__
#define __ARM__

#include "Enemy.h"
#include "StateMachine.h"

typedef struct {
	Entity* player;
	float			turnTime;
	float			aggroTime;
} ArmWanderData;

typedef struct {
	Entity*			player;
	float			attackCheckTime;
	float			turnTime;
} ArmChaseData;

typedef struct {
	Entity* player;
	Bool			attacking;
	float			attackStartupTime;
} ArmAttackData;

Entity* createArm(Entity* player);

void armWanderEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void armWanderThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void armWanderUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void armWanderOnHit(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);

void armChaseEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void armChaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void armChaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);

void armAttackEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine);
void armAttackThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);
void armAttackUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine);


#endif