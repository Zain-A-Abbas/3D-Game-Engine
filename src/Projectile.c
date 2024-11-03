#include "Projectile.h"
#include "simple_logger.h"
#include "gf3d_draw.h"

const char *BASE_FILENAME = "models/projectiles/";
const float INVISIBLE_TIME = 0.1;

Entity* newProjectile(Projectile* data, const char* filename) {
	Entity* projectile = entityNew();
	if (!projectile) {
		slog("Failed to create projectile");
		return NULL;
	}
	const char* fileLocation = malloc(strlen(BASE_FILENAME) + strlen(filename) + strlen(".model") + 1);
	strcpy(fileLocation, BASE_FILENAME);
	strcat(fileLocation, filename);
	strcat(fileLocation, ".model");

	Model* projectileModel = gf3d_model_load(fileLocation);
	if (!projectileModel) {
		slog("Failed to create projectile model.");
		projectileDelete(projectile);
		return NULL;
	}

	projectile->think = projectileThink;
	projectile->update = projectileUpdate;
	projectile->model = projectileModel;
	projectile->data = data;
	projectile->type = PROJECTILE;
	return projectile;
}

void projectileDelete(Entity* self) {
	free(self->data);
	_entityFree(self);
}

void projectileThink(Entity* self, float delta) {
	Projectile* data = (Projectile*)self->data;
	data->lifetime += delta;
	if (data->lifetime >= INVISIBLE_TIME && self->scale.x == 0) {
		self->scale = gfc_vector3d(1, 1, 1);
	}
	if (data->lifetime >= data->maxLifetime) {
		projectileDelete(self);
	}

}

void projectileUpdate(Entity* self, float delta) {
	Projectile* data = (Projectile*)self->data;

	self->position.x += data->velocity.x * delta;
	self->position.y += data->velocity.y * delta;
	self->position.z += data->velocity.z * delta;


	GFC_Vector3D normalizedVelocity = data->velocity;
	gfc_vector3d_normalize(&normalizedVelocity);
	data->raycast.a = self->position;
	data->raycast.b = gfc_vector3d_added(self->position, gfc_vector3d(normalizedVelocity.x * 4.0, normalizedVelocity.y * 4.0, normalizedVelocity.z * 4.0));

	GFC_Triangle3D t = { 0 };

	
	for (int i = 0; i < entityManager.entityMax; i++) {
		// Filter out inactive entities, non-collideable, and collideable out of range
		Entity* currEntity = &entityManager.entityList[i];
		if (!currEntity->_in_use) {
			continue;
		}
		if (!isOnLayer(currEntity, 1) && !isOnLayer(currEntity, 2)) {
			continue;
		}
		if (!gfc_vector3d_distance_between_less_than(entityGlobalPosition(self), entityGlobalPosition(currEntity), 128)) {
			continue;
		}

		if (entityRaycastTest(currEntity, data->raycast, NULL, &t, NULL)) {
			entityAttacked(currEntity, data->damage);
			projectileDelete(self);
			return;
		}
	}

	
}