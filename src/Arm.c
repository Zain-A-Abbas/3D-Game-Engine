#include "Arm.h"
#include "simple_logger.h"
#include "TypesExtra.h"
#include "Player.h"

const int ARM_HP = 45;
const float ARM_AGGRO_RANGE = 50;

const float ARM_WANDER_SPEED = 12;
const float ARM_WANDER_TURN_INTERVAL = 4.0;
const float ARM_WANDER_AGGRO_INTERVAL = 0.2;

const float ARM_CHASE_SPEED = 24;
const float ARM_CHASE_TURN_SPEED = 32;
const float ARM_CHASE_TURN_INTERVAL = 0.04;
const float ARM_CHASE_ATTACK_INTERVAL = 0.12;

const float ARM_ATTACK_TURN_SPEED = 0;
const float ARM_ATTACK_STARTUP = 0.6;
const float ARM_ATTACK_COOLDOWN = 2;
const int ARM_MELEE_DAMAGE = 24;

Entity* createArm(Entity* player) {
	Entity* newArm = enemyEntityNew(player);
	if (!newArm) {
		return;
	}

	newArm->parent = NULL;
	newArm->scale = gfc_vector3d(10, 10, 10);

	enemySetCollision(newArm, 18, 6);

	EnemyData* enemyData = (EnemyData*)newArm->data;
	enemyData->hp = ARM_HP;
	enemyData->character3dData->walkSoundTime = 0.2;

	// Animation/Models

	animationSetup(
		newArm,
		"models/enemies/arm/",
		(char* []) {
			"ArmRun",
			"ArmAttack",
			"ArmDeath",
			NULL
		}
	);
	animationPlay(newArm, "ArmRun", true);

	// AI

	// Make and assign states
	StateMachine* stateMachine = createStateMachine();
	enemyData->enemyStateMachine = stateMachine;

	State* wanderState = createState("Wander", stateMachine, armWanderEnter, NULL, armWanderThink, armWanderUpdate, armWanderOnHit, calloc(1, sizeof(ArmWanderData)));
	ArmWanderData* wanderData = (ArmWanderData*)wanderState->stateData;
	wanderData->player = player;

	State* chaseState = createState("Chase", stateMachine, armChaseEnter, NULL, armChaseThink, armChaseUpdate, NULL, calloc(1, sizeof(ArmChaseData)));
	ArmChaseData* chaseData = (ArmChaseData*)chaseState->stateData;
	chaseData->player = player;

	State* attackState = createState("Attack", stateMachine, armAttackEnter, NULL, armAttackThink, armAttackUpdate, NULL, calloc(1, sizeof(ArmAttackData)));
	ArmAttackData* attackData = (ArmAttackData*)attackState->stateData;
	attackData->player = player;

	giveDeathState(stateMachine, "ArmDeath", player);

	changeState(newArm, stateMachine, "Wander");

	return newArm;
}


// WANDER

void armWanderEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	//printf("\nHELLO MOTHERFUCKERS!");
	animationPlay(self, "ArmRun", true);
	EnemyData* enemyData = (EnemyData*)self->data;
}

void armWanderUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData, delta);
}

void armWanderThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	ArmWanderData* wanderData = (ArmWanderData*)state->stateData;
	wanderData->aggroTime += delta;
	wanderData->turnTime += delta;

	if (wanderData->aggroTime >= ARM_WANDER_AGGRO_INTERVAL) {
		if (gfc_vector3d_distance_between_less_than(entityGlobalPosition(wanderData->player), entityGlobalPosition(self), ARM_AGGRO_RANGE)) {
			changeState(self, stateMachine, "Chase");
			return;
		}
	}

	if (wanderData->turnTime >= ARM_WANDER_TURN_INTERVAL) {
		wanderData->turnTime = 0;
		enemyData->character3dData->rotation.z = gfc_random() * GFC_HALF_PI * 2 - GFC_HALF_PI; // Rotate by up to 90 degrees in either direction randomly
		enemyData->character3dData->motionVelocity = gfc_vector3d(0, -ARM_WANDER_SPEED, 0);
		gfc_vector3d_rotate_about_z(&enemyData->character3dData->motionVelocity, enemyData->character3dData->rotation.z);
	}

}


void armWanderOnHit(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	changeState(self, stateMachine, "Chase");
}

// CHASE

void armChaseEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	animationPlay(self, "ArmRun", true);
	EnemyData* enemyData = (EnemyData*)self->data;
}

void armChaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData, delta);
}

void armChaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	ArmChaseData* chaseData = (ArmChaseData*)state->stateData;
	chaseData->attackCheckTime += delta;
	chaseData->turnTime += delta;
	if (chaseData->turnTime >= ARM_CHASE_TURN_INTERVAL) {

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

		float rotationAmount = ARM_CHASE_TURN_SPEED;

		if (fabsf(angleDifference) <= rotationAmount) {
			enemyData->character3dData->rotation.z = angleTarget + GFC_HALF_PI;
		}
		else if (angleDifference > 0) {
			enemyData->character3dData->rotation.z += rotationAmount * delta;

		}
		else if (angleDifference < 0) {
			enemyData->character3dData->rotation.z -= rotationAmount * delta;
		}

		if (fabsf(enemyData->character3dData->rotation.z) > GFC_2PI) {
			enemyData->character3dData->rotation.z = fMoveTowards(enemyData->character3dData->rotation.z, 0.0, GFC_2PI);
		}


		if (fabsf(angleDifference) < GFC_PI / 16.0) {
			if (enemyData->character3dData->motionVelocity.x + enemyData->character3dData->motionVelocity.y == 0) {
				animationPlay(self, "ArmRun", true);
			}
			enemyData->character3dData->motionVelocity = gfc_vector3d(0, -ARM_CHASE_SPEED, 0);
			gfc_vector3d_rotate_about_z(&enemyData->character3dData->motionVelocity, self->rotation.z);
		}

	}

	if (chaseData->attackCheckTime >= ARM_CHASE_ATTACK_INTERVAL) {

		// Attack
		if (fabsf(entityDirectionTo(self, chaseData->player)) < 0.3 && gfc_vector3d_magnitude_between(entityGlobalPosition(self), entityGlobalPosition(chaseData->player)) < 12) {
			changeState(self, stateMachine, "Attack");
		}
	}
}

// ATTACK

void armAttackEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	animationPlay(self, "ArmAttack", false);
	EnemyData* enemyData = (EnemyData*)self->data;
	ArmAttackData* attackData = (ArmAttackData*)state->stateData;
	attackData->attacking = false;
	attackData->attackStartupTime = 0;
}

void armAttackUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData, delta);
}

void armAttackThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	ArmAttackData* attackData = (ArmAttackData*)state->stateData;
	attackData->attackStartupTime += delta;
	if (attackData->attackStartupTime > ARM_ATTACK_STARTUP && !attackData->attacking) {
		attackData->attacking = true;
		GFC_Sphere attackSphere = { 0 };
		GFC_Vector3D attackPosition = entityGlobalPosition(self);
		GFC_Vector3D attackOffset = gfc_vector3d(2, -7, 0);
		gfc_vector3d_rotate_about_z(&attackOffset, entityGlobalRotation(self).z);
		attackPosition = gfc_vector3d_added(attackPosition, attackOffset);
		attackSphere.x = attackPosition.x; attackSphere.y = attackPosition.y; attackSphere.z = attackPosition.z;
		attackSphere.r = 8;
		enemyData->attackSphere = attackSphere;
		if (gfc_point_in_sphere(entityGlobalPosition(attackData->player), attackSphere)) {
			playerTakeDamage(attackData->player, ARM_MELEE_DAMAGE);
		}
	}

	if (self->entityAnimation->animationFinished) {
		changeState(self, stateMachine, "Chase");
	}

}