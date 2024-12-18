#include "simple_logger.h"
#include "gfc_matrix.h"
#include "Enemy.h"
#include "Player.h"
#include "AmmoPickup.h"
#include "Level.h"

const Uint8 ENEMY_LAYERS = 0b00000100;


Entity * enemyEntityNew(Entity *player) {
	Entity* enemyEntity = entityNew();
	if (enemyEntity == NULL) {
		slog("Enemy Entity could not be created");
		return NULL;
	}
	enemyEntity->think = enemyThink;
	enemyEntity->update = enemyUpdate;
	enemyEntity->type = ENEMY;


	EnemyData* enemyData = (EnemyData*)malloc(sizeof(EnemyData));
    if (!enemyData) {
        slog("Failed to allocate memory for enemy data.");
		gf3d_model_free(enemyEntity->model);
		free(enemyEntity);
        return NULL;
    }
	memset(enemyData, 0, sizeof(enemyData));
	enemyEntity->data = enemyData;
	
	enemyData->player = player;
	enemyData->character3dData = newCharacter3dData();
	enemyData->attackSphere = gfc_sphere(0, 0, 0, 0);

	enemyEntity->collisionLayer = ENEMY_LAYERS;
	return enemyEntity;
};

void enemyThink(Entity* self, float delta) {
	if (entityManager.disableEnemies) {
		return;
	}
	EnemyData* enemyData = (EnemyData*)self->data;
	if (enemyData->enemyStateMachine) {
		enemyData->enemyStateMachine->currentState->think(self, delta, enemyData->enemyStateMachine->currentState, enemyData->enemyStateMachine);
	}
	return;
}


void enemyUpdate(Entity* self, float delta) {
	if (entityManager.disableEnemies) {
		return;
	}
	EnemyData* enemyData = (EnemyData*)self->data;
	if (enemyData->enemyStateMachine) {
		if (enemyData->enemyStateMachine->currentState) {
			enemyData->enemyStateMachine->currentState->update(self, delta, enemyData->enemyStateMachine->currentState, enemyData->enemyStateMachine);
		}
	}
	return;
}

void enemyAttacked(Entity* self, int damage) {
	printf("\nHit for %d", damage);
	EnemyData* enemyData = (EnemyData*)self->data;
	enemyData->hp -= damage;
	if (enemyData->hp <= 0) {
		changeState(self, enemyData->enemyStateMachine, "Dying");
		return;
	}
	if (enemyData->enemyStateMachine) {
		if (enemyData->enemyStateMachine->currentState) {
			if (enemyData->enemyStateMachine->currentState->onHit) {
				enemyData->enemyStateMachine->currentState->onHit(self, enemyData->enemyStateMachine->currentState, enemyData->enemyStateMachine);
			}
		}
	}
}

void giveDeathState(StateMachine* stateMachine, char dyingAnimation[32], Entity *player) {
	State* deathState = createState("Dying", stateMachine, dyingEnter, NULL, dyingThink, dyingUpdate, NULL, calloc(1, sizeof(DyingData)));
	DyingData* dyingData = (DyingData*)deathState->stateData;
	dyingData->player = player;
	strcpy(dyingData->dyingAnimName, dyingAnimation);
}

void dyingEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	self->collisionLayer = 0b00000000;
	DyingData* dyingData = (DyingData*)state->stateData;
	PlayerData* playerData = (PlayerData*)dyingData->player->data;
	playerData->money += 5 + gfc_random_int(10);
	animationPlay(self, dyingData->dyingAnimName, false);

	// Ammo pickup
	Entity* ammoPickup = createAmmoPickup(dyingData->player);
	ammoPickup->position = self->position;
	ammoPickup->position.z += 4;
}

void dyingThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	if (self->entityAnimation) {
		if (self->entityAnimation->animationFinished) {
			levelEnemyKilled();
			enemyDelete(self);
		}
	}
}

void dyingUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {

}


void enemyDelete(Entity* self) {
	EnemyData *enemyData = (EnemyData*)self->data;
	if (enemyData) {
		if (enemyData->enemyStateMachine) {
			stateMachineFree(enemyData->enemyStateMachine);
		}

		if (enemyData->character3dData) {
			free(enemyData->character3dData);
		}
	}
	_entityFree(self);
}

void enemySetCollision(Entity* self, float height, float radius) {
	// Collision
	EntityCollision* collision = (EntityCollision*)malloc(sizeof(EntityCollision));
	memset(collision, 0, sizeof(EntityCollision));

	// Create capsule
	GFC_ExtendedPrimitive* collisionPrimitive = (GFC_ExtendedPrimitive*)malloc(sizeof(GFC_ExtendedPrimitive));
	memset(collisionPrimitive, 0, sizeof(GFC_ExtendedPrimitive));
	collisionPrimitive->type = E_Capsule;


	GFC_Capsule enemyCapsule = gfc_capsule(height, radius);
	collisionPrimitive->s.c = enemyCapsule;
	collision->collisionPrimitive = collisionPrimitive;

	GFC_Box boundingBox = { 0 };
	boundingBox.w = enemyCapsule.radius * 4;
	boundingBox.d = enemyCapsule.height * 4;
	boundingBox.h = enemyCapsule.radius * 4;

	collision->AABB = boundingBox;

	self->entityCollision = collision;
}