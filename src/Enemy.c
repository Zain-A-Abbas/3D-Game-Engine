#include "simple_logger.h"
#include "gfc_matrix.h"
#include "Enemy.h"

const Uint8 ENEMY_LAYERS = 0b00000100;

Entity * enemyEntityNew() {
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
	enemyEntity->data = enemyData;
	memset(enemyData, 0, sizeof(enemyData));
	enemyData->aiTime = 0.0;
	

	enemyData->character3dData = newCharacter3dData();

	enemyEntity->collisionLayer = ENEMY_LAYERS;
	return enemyEntity;
};

void enemyThink(Entity* self, float delta) {
	EnemyData* enemyData = (EnemyData*)self->data;
	if (enemyData->enemyStateMachine) {
		enemyData->enemyStateMachine->currentState->think(self, delta, enemyData->enemyStateMachine->currentState, enemyData->enemyStateMachine);
	}
	return;
}


void enemyUpdate(Entity* self, float delta) {
	EnemyData* enemyData = (EnemyData*)self->data;
	if (enemyData->enemyStateMachine) {
		enemyData->enemyStateMachine->currentState->update(self, delta, enemyData->enemyStateMachine->currentState, enemyData->enemyStateMachine);
	}
	return;
}

void enemyAttacked(Entity* self, int damage) {
	printf("\nHit for %d", damage);
	EnemyData* enemyData = (EnemyData*)self->data;
	enemyData->hp -= damage;
	if (enemyData->hp <= 0) {
		enemyDelete(self);
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

void enemyDelete(Entity* self) {
	EnemyData* enemyData = (EnemyData*)self->data;
	if (enemyData->enemyStateMachine) {
		stateMachineFree(enemyData->enemyStateMachine);
	}
	if (enemyData->enemyCollision) {
		gf3d_model_free(enemyData->enemyCollision);
	}
	if (enemyData->character3dData) {
		free(enemyData->character3dData);
	}
	_entityFree(self);
}

void enemyScalePreserveModel(Entity* self, GFC_Vector3D scale) {
	EnemyData* enemyData = (EnemyData*)self->data;
	self->scale = gfc_vector3d_multiply(self->scale, scale);
	GFC_Vector3D modelScale = gfc_vector3d(1.0 / scale.x, 1.0 / scale.y, 1.0 / scale.z);
	enemyData->enemyCollision->matrix[0][0] *= modelScale.x;
	enemyData->enemyCollision->matrix[1][1] *= modelScale.y;
	enemyData->enemyCollision->matrix[2][2] *= modelScale.z;
	enemyData->enemyCollision->matrix[3][3] = 1;

}