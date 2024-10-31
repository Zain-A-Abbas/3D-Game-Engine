#include "Zombie.h"
#include "simple_logger.h"

Entity* createZombie() {
	Entity* newZombie = enemyEntityNew();
	animationSetup(newZombie, "models/enemies/zombie/");
	animationPlay(newZombie, "ZombiePunch");

	StateMachine* stateMachine = (StateMachine*)malloc(sizeof(StateMachine));
	if (!stateMachine) {
		slog("Could not allocate enemy state machine");
		free(newZombie);
		return NULL;

	}
	memset(stateMachine, 0, sizeof(StateMachine));

	stateMachine->stateList = gfc_list_new();
	State* wanderState = createState("Wander", NULL, NULL, wanderThink, wanderUpdate);

	gfc_list_append(stateMachine->stateList, wanderState);

	stateMachine->currentState = gfc_list_get_nth(stateMachine->stateList, 0);


	EnemyData* enemyData = (EnemyData*)newZombie->data;
	enemyData->enemyStateMachine = stateMachine;
	return newZombie;
}

void wanderUpdate(struct Entity_S* self, float delta) {

	self->position.y -= 0.1;
}

void wanderThink(struct Entity_S* self, float delta) {

}