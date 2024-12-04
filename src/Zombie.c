#include "Zombie.h"
#include "simple_logger.h"
#include "TypesExtra.h"
#include "Player.h"

const int HP = 30;
const float AGGRO_RANGE = 40;

const float WANDER_SPEED = 4;
const float WANDER_AI_INTERVAL = 4.0;
const float WANDER_TURN_INTERVAL = 4.0;
const float WANDER_AGGRO_INTERVAL = 0.2;

const float CHASE_SPEED = 7;
const float CHASE_TURN_SPEED = 8;
const float CHASE_ATTACK_INTERVAL = 0.12;

const float ATTACK_TURN_SPEED = 4;
const float ATTACK_STARTUP = 0.25;
const float ATTACK_COOLDOWN = 2;
const int MELEE_DAMAGE = 16;

Entity* createZombie(Entity* player) {
	Entity* newZombie = enemyEntityNew();
	if (!newZombie) {
		return;
	}

	newZombie->parent = NULL;
	newZombie->scale = gfc_vector3d(10, 10, 10);

	enemySetCollision(newZombie, 18, 4.5);

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
	WanderData* wanderData = (WanderData*)wanderState->stateData;
	wanderData->player = player;
	
	State* chaseState = createState("Chase", stateMachine, chaseEnter, NULL, chaseThink, chaseUpdate, NULL, calloc(1, sizeof(ChaseData)));
	ChaseData* chaseData = (ChaseData*)chaseState->stateData;
	chaseData->player = player;

	State* attackState = createState("Attack", stateMachine, attackEnter, NULL, attackThink, attackUpdate, NULL, calloc(1, sizeof(AttackData)));
	AttackData* attackData = (AttackData*)attackState->stateData;
	attackData->player = player;

	giveDeathState(stateMachine, "ZombieDeath");

	changeState(newZombie, stateMachine, "Wander");

	return newZombie;
}


// WANDER

void wanderEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	//printf("\nHELLO MOTHERFUCKERS!");
	animationPlay(self, "ZombieWalk", true);
	EnemyData* enemyData = (EnemyData*)self->data;
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
	wanderData->aggroTime += delta;
	wanderData->turnTime += delta;

	if (wanderData->aggroTime >= WANDER_AGGRO_INTERVAL) {
		if (gfc_vector3d_distance_between_less_than(entityGlobalPosition(wanderData->player), entityGlobalPosition(self), AGGRO_RANGE)) {
			changeState(self, stateMachine, "Chase");
			return;
		}
	}

	if (wanderData->turnTime >= WANDER_TURN_INTERVAL) {
		wanderData->turnTime = 0;
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
	chaseData->attackCheckTime += delta;
	if (chaseData->attackCheckTime >= CHASE_ATTACK_INTERVAL) {

		chaseData->attackCheckTime = 0;

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

		// Attack
		if (fabsf(entityDirectionTo(self, chaseData->player)) < 0.3 && gfc_vector3d_magnitude_between( entityGlobalPosition(self), entityGlobalPosition(chaseData->player) ) < 12) {
			changeState(self, stateMachine, "Attack");
		}
	}
}

// ATTACK

void attackEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	animationPlay(self, "ZombieAttack", false);
	EnemyData* enemyData = (EnemyData*)self->data;
	AttackData* attackData = (AttackData*)state->stateData;
	attackData->attacking = false;
	attackData->attackStartupTime = 0;
}

void attackUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
}

void attackThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	AttackData* attackData = (AttackData*)state->stateData;
	attackData->attackStartupTime += delta;
	if (attackData->attackStartupTime > ATTACK_STARTUP && !attackData->attacking) {
		attackData->attacking = true;
		GFC_Sphere attackSphere = {0};
		GFC_Vector3D attackPosition = entityGlobalPosition(self);
		GFC_Vector3D attackOffset = gfc_vector3d(0, -3.5, 0);
		gfc_vector3d_rotate_about_z(&attackOffset, entityGlobalRotation(self).z);
		attackPosition = gfc_vector3d_added(attackPosition, attackOffset);
		attackSphere.x = attackPosition.x; attackSphere.y = attackPosition.y; attackSphere.z = attackPosition.z;
		attackSphere.r = 6;
		enemyData->attackSphere = attackSphere;
		if (gfc_point_in_sphere(entityGlobalPosition(attackData->player), attackSphere)) {
			playerTakeDamage(attackData->player, MELEE_DAMAGE);
		}
	}

	if (self->entityAnimation->animationFinished) {
		changeState(self, stateMachine, "Chase");
	}

}