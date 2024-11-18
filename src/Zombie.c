#include "Zombie.h"
#include "simple_logger.h"
#include "TypesExtra.h"

const int HP = 30;
const float AGGRO_RANGE = 8;

const float WANDER_SPEED = 4;
const float WANDER_AI_INTERVAL = 4.0;

const float CHASE_SPEED = 7;
const float CHASE_TURN_SPEED = 8;
const float CHASE_AI_INTERVAL = 0.12;

Entity* createZombie(Entity* player) {
	Entity* newZombie = enemyEntityNew();
	if (!newZombie) {
		return;
	}

	newZombie->scale = gfc_vector3d(2, 2, 2);

	enemySetCollision(newZombie, 8, 2);

	EnemyData* enemyData = (EnemyData*)newZombie->data;
	enemyData->hp = HP;

	// Animation/Models

	animationSetup(
        newZombie,
        "models/enemies/zombie/", 
        (char *[]){
			"ZombieIdle",
			"ZombieWalk",
			"ZombieAttack",
			"ZombieDeath",
			NULL
        }
    );
	animationPlay(newZombie, "ZombieWalk", true);

	// AI
		// Make and assign states
	StateMachine* stateMachine = createStateMachine();
	enemyData->enemyStateMachine = stateMachine;

	State* wanderState = createState("Wander", stateMachine, wanderEnter, NULL, wanderThink, wanderUpdate, wanderOnHit, calloc(1, sizeof(WanderData)));
	State* chaseState = createState("Chase", stateMachine, chaseEnter, NULL, chaseThink, chaseUpdate, NULL, calloc(1, sizeof(ChaseData)));
	State* deathState = createState("Dying", stateMachine, dyingEnter, NULL, dyingThink, dyingUpdate, NULL, calloc(1, sizeof(DyingData)));
	ChaseData* chaseData = (ChaseData*)chaseState->stateData;
	chaseData->player = player;
	WanderData* wanderData = (WanderData*)wanderState->stateData;
	wanderData->player = player;
	DyingData* dyingData = (DyingData*)deathState->stateData;
	strcpy(dyingData->dyingAnimName, "ZombieDeath");

	changeState(newZombie, stateMachine, "Wander");


	return newZombie;
}


// WANDER

void wanderEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	//printf("\nHELLO MOTHERFUCKERS!");
	animationPlay(self, "ZombieWalk", true);
	EnemyData* enemyData = (EnemyData*)self->data;
	enemyData->aiTime = WANDER_AI_INTERVAL;
}

void wanderUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData, delta);
}

void wanderThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	WanderData* wanderData = (WanderData*)state->stateData;
	enemyData->aiTime += delta;

	if (enemyData->aiTime >= WANDER_AI_INTERVAL) {
		if (gfc_vector3d_distance_between_less_than(entityGlobalPosition(wanderData->player), entityGlobalPosition(self), AGGRO_RANGE)) {
			changeState(self, stateMachine, "Chase");
			return;
		}
		enemyData->aiTime = 0;
		enemyData->character3dData->rotation.z = gfc_random() * GFC_HALF_PI * 2 - GFC_HALF_PI; // Rotate by up to 90 degrees in either direction randomly
		enemyData->character3dData->velocity = gfc_vector3d(0, -WANDER_SPEED, 0);
		gfc_vector3d_rotate_about_z(&enemyData->character3dData->velocity, enemyData->character3dData->rotation.z);
	}
}


void wanderOnHit(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	changeState(self, stateMachine, "Chase");
}

// CHASE

void chaseEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	animationPlay(self, "ZombieWalk", true);
	EnemyData* enemyData = (EnemyData*)self->data;
	enemyData->aiTime = CHASE_AI_INTERVAL;
}

void chaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData, delta);
}

void chaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	ChaseData* chaseData = (ChaseData*)state->stateData;
	enemyData->aiTime += delta;
	if (enemyData->aiTime >= CHASE_AI_INTERVAL) {

		enemyData->aiTime = 0;

		GFC_Vector2D positionDifference = gfc_vector2d(chaseData->player->position.x - self->position.x, chaseData->player->position.y - self->position.y);
		gfc_vector2d_normalize(&positionDifference);
		float angleTarget = gfc_vector2d_angle(gfc_vector2d(positionDifference.x, positionDifference.y)) - GFC_HALF_PI;
		float angleDifference = angleTarget - enemyData->character3dData->rotation.z + GFC_HALF_PI;
		while (angleDifference < -GFC_PI_HALFPI) {
			angleDifference += GFC_2PI;
		}
		while (angleDifference > GFC_HALF_PI) {
			angleDifference -= GFC_2PI;
		}

		float rotationAmount = CHASE_TURN_SPEED;

		if (fabsf(angleDifference) <= rotationAmount) {
			enemyData->character3dData->rotation.z = angleTarget + GFC_HALF_PI;
		} else if (angleDifference > 0) {
			enemyData->character3dData->rotation.z += rotationAmount * delta;

		}
		else if (angleDifference < 0) {
			enemyData->character3dData->rotation.z -= rotationAmount * delta;
		}

		if (fabsf(enemyData->character3dData->rotation.z) > GFC_2PI) {
			enemyData->character3dData->rotation.z = fMoveTowards(enemyData->character3dData->rotation.z, 0.0, GFC_2PI);
		}
		

		if (fabsf(angleDifference) < GFC_PI/16.0) {
			if (enemyData->character3dData->velocity.x + enemyData->character3dData->velocity.y == 0) {
				animationPlay(self, "ZombieWalk", true);
			}
			enemyData->character3dData->velocity = gfc_vector3d(0, -CHASE_SPEED, 0);
			gfc_vector3d_rotate_about_z(&enemyData->character3dData->velocity, enemyData->character3dData->rotation.z);
		}
		else {
			if (enemyData->character3dData->velocity.x + enemyData->character3dData->velocity.y != 0) {
				animationPlay(self, "ZombieIdle", true);
			}
			enemyData->character3dData->velocity = gfc_vector3d(0, 0, 0);
		}
	}
}