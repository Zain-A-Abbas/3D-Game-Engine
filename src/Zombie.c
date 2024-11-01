#include "Zombie.h"
#include "simple_logger.h"
#include "TypesExtra.h"

const float WANDER_SPEED = 0.125;
const float WANDER_AI_INTERVAL = 0.12;
const float CHASE_SPEED = 5;
const float CHASE_TURN_SPEED = 8;

Entity* createZombie(Entity *player) {
	Entity* newZombie = enemyEntityNew();
	animationSetup(newZombie, "models/enemies/zombie/");
	animationPlay(newZombie, "ZombieIdle");

	StateMachine* stateMachine = (StateMachine*)malloc(sizeof(StateMachine));
	if (!stateMachine) {
		slog("Could not allocate enemy state machine");
		free(newZombie);
		return NULL;

	}
	memset(stateMachine, 0, sizeof(StateMachine));

	State *chaseState = createState("Wander", stateMachine, NULL, NULL, chaseThink, chaseUpdate, calloc(1, sizeof(ChaseData)));
	ChaseData *chaseData = (ChaseData*)chaseState->stateData;
	chaseData->player = player;

	changeState(newZombie, stateMachine, "Wander");


	EnemyData* enemyData = (EnemyData*)newZombie->data;
	enemyData->enemyStateMachine = stateMachine;


	return newZombie;
}


void chaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData);
	printf("\nThe vertical height is: %f", self->position.z);
}

void chaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	WanderData* wanderData = (WanderData*)state->stateData;
	enemyData->aiTime += delta;
	if (enemyData->aiTime >= WANDER_AI_INTERVAL) {
		enemyData->aiTime = 0;

		GFC_Vector2D positionDifference = gfc_vector2d(wanderData->player->position.x - self->position.x, wanderData->player->position.y - self->position.y);
		gfc_vector2d_normalize(&positionDifference);
		float angleTarget = gfc_vector2d_angle(gfc_vector2d(positionDifference.x, positionDifference.y)) - GFC_HALF_PI;
		float angleDifference = angleTarget - enemyData->character3dData->rotation.z + GFC_HALF_PI;
		while (angleDifference < -GFC_PI_HALFPI) {
			angleDifference += GFC_2PI;
		}
		while (angleDifference > GFC_HALF_PI) {
			angleDifference -= GFC_2PI;
		}

		float rotationAmount = CHASE_TURN_SPEED * delta;

		//printf("\nAngle target: %f", angleTarget);
		//printf("\nAngle diff: %f", angleDifference);
		//printf("\nCurrent rotation: %f", enemyData->character3dData->rotation.z);
		
		if (fabsf(angleDifference) <= rotationAmount) {
			enemyData->character3dData->rotation.z = angleTarget + GFC_HALF_PI;
		} else if (angleDifference > 0) {
			enemyData->character3dData->rotation.z += rotationAmount;

		}
		else if (angleDifference < 0) {
			enemyData->character3dData->rotation.z -= rotationAmount;
		}

		if (fabsf(enemyData->character3dData->rotation.z) > GFC_2PI) {
			enemyData->character3dData->rotation.z = fMoveTowards(enemyData->character3dData->rotation.z, 0.0, GFC_2PI);
		}
		

		if (fabsf(angleDifference) < GFC_PI/32.0) {
			enemyData->character3dData->velocity = gfc_vector3d(0, -CHASE_SPEED * delta, 0);
			gfc_vector3d_rotate_about_z(&enemyData->character3dData->velocity, enemyData->character3dData->rotation.z);
		}
		else {
			enemyData->character3dData->velocity = gfc_vector3d(0, 0, 0);
		}
	}
}