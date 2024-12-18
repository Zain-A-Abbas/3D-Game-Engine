#include "Ghost.h"
#include "simple_logger.h"
#include "TypesExtra.h"
#include "Player.h"

const int GHOST_HP = 40;

const float GHOST_CHASE_SPEED = 7;
const float GHOST_CHASE_TURN_SPEED = 6;
const float GHOST_CHASE_ATTACK_INTERVAL = 0.12;

const float GHOST_ATTACK_TURN_SPEED = 4;
const float GHOST_ATTACK_STARTUP = 0.25;
const float GHOST_ATTACK_COOLDOWN = 2;
const int GHOST_MELEE_DAMAGE = 12;

Entity* createGhost(Entity* player) {
	Entity* newGhost = enemyEntityNew(player);
	if (!newGhost) {
		return;
	}

	newGhost->parent = NULL;
	newGhost->scale = gfc_vector3d(10, 10, 10);
	newGhost->draw = ghostDraw;

	enemySetCollision(newGhost, 18, 4.5);

	EnemyData* enemyData = (EnemyData*)newGhost->data;
	enemyData->hp = GHOST_HP;

	// Animation/Models

	animationSetup(
		newGhost,
		"models/enemies/ghost/",
		(char* []) {
			"GhostIdle",
			"GhostAttack",
			"GhostDeath",
			NULL
		}
	);
	animationPlay(newGhost, "GhostIdle", true);

	// AI

	// Make and assign states
	StateMachine* stateMachine = createStateMachine();
	enemyData->enemyStateMachine = stateMachine;


	State* chaseState = createState("Chase", stateMachine, ghostChaseEnter, NULL, ghostChaseThink, ghostChaseUpdate, NULL, calloc(1, sizeof(GhostChaseData)));
	GhostChaseData* chaseData = (GhostChaseData*)chaseState->stateData;
	chaseData->player = player;

	State* attackState = createState("Attack", stateMachine, ghostAttackEnter, NULL, ghostAttackThink, ghostAttackUpdate, NULL, calloc(1, sizeof(GhostAttackData)));
	GhostAttackData* attackData = (GhostAttackData*)attackState->stateData;
	attackData->player = player;

	giveDeathState(stateMachine, "GhostDeath", player);

	changeState(newGhost, stateMachine, "Chase");

	return newGhost;
}

// CHASE

void ghostChaseEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	animationPlay(self, "GhostIdle", true);
	EnemyData* enemyData = (EnemyData*)self->data;
}

void ghostChaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	self->position = gfc_vector3d_added(self->position, gfc_vector3d(enemyData->character3dData->motionVelocity.x * delta, enemyData->character3dData->motionVelocity.y * delta, enemyData->character3dData->motionVelocity.z * delta));
	//moveAndSlide(self, enemyData->character3dData, delta);
}

void ghostChaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	GhostChaseData* chaseData = (GhostChaseData*)state->stateData;
	chaseData->attackCheckTime += delta;
	if (chaseData->attackCheckTime >= GHOST_CHASE_ATTACK_INTERVAL) {

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

		float rotationAmount = GHOST_CHASE_TURN_SPEED;

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


		enemyData->character3dData->motionVelocity = gfc_vector3d(0, -GHOST_CHASE_SPEED, -25 * delta * (chaseData->player->position.z < self->position.z));
		gfc_vector3d_rotate_about_z(&enemyData->character3dData->motionVelocity, enemyData->character3dData->rotation.z);

		// Attack
		if (fabsf(entityDirectionTo(self, chaseData->player)) < 0.3 && gfc_vector3d_magnitude_between(entityGlobalPosition(self), entityGlobalPosition(chaseData->player)) < 12) {
			changeState(self, stateMachine, "Attack");
		}
	}
}

// ATTACK

void ghostAttackEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	animationPlay(self, "GhostAttack", false);
	EnemyData* enemyData = (EnemyData*)self->data;
	GhostAttackData* attackData = (GhostAttackData*)state->stateData;
	attackData->attacking = false;
	attackData->attackStartupTime = 0;
}

void ghostAttackUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
}

void ghostAttackThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	GhostAttackData* attackData = (GhostAttackData*)state->stateData;
	attackData->attackStartupTime += delta;
	if (attackData->attackStartupTime > GHOST_ATTACK_STARTUP && !attackData->attacking) {
		attackData->attacking = true;
		GFC_Sphere attackSphere = { 0 };
		GFC_Vector3D attackPosition = entityGlobalPosition(self);
		GFC_Vector3D attackOffset = gfc_vector3d(0, -3.5, 0);
		gfc_vector3d_rotate_about_z(&attackOffset, entityGlobalRotation(self).z);
		attackPosition = gfc_vector3d_added(attackPosition, attackOffset);
		attackSphere.x = attackPosition.x; attackSphere.y = attackPosition.y; attackSphere.z = attackPosition.z;
		attackSphere.r = 6;
		enemyData->attackSphere = attackSphere;
		if (gfc_point_in_sphere(entityGlobalPosition(attackData->player), attackSphere)) {
			playerTakeDamage(attackData->player, GHOST_MELEE_DAMAGE);
		}
	}

	if (self->entityAnimation->animationFinished) {
		changeState(self, stateMachine, "Chase");
	}
}

void* ghostDraw(Entity* self, LightUBO* lights) {
	if (!self) return;

	GFC_Matrix4 matrix;
	gfc_matrix4_from_vectors(
		matrix,
		entityGlobalPosition(self),
		entityGlobalRotation(self),
		entityGlobalScale(self)
	);

	float alpha = 1.0;
	EnemyData* enemyData = (EnemyData*)self->data;
	StateMachine* stateMachine = enemyData->enemyStateMachine;
	if (strcmp(stateMachine->currentState->stateName, "Chase") == 0) {
		GhostChaseData* chaseData = (GhostChaseData*)stateMachine->currentState->stateData;
		chaseData->alpha += 0.0125;
		alpha = 0.3 + 0.3 * fabsf(sinf(chaseData->alpha));
	}

	gf3d_model_draw(
		self->model,
		matrix,
		gfc_color(1, 1, 1, alpha),
		NULL,
		0
	);
}
