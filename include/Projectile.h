#ifndef __PROJECTILE__
#define __PROJECTILE__
#include "gfc_types.h"
#include "gfc_vector.h"
#include "Entity.h"

typedef struct Projectile_S {
	GFC_Vector3D		velocity;
	float				maxLifetime;
	float				lifetime;
	int					damage;
	Uint8				layers;
	GFC_Edge3D			raycast;
} Projectile;

Entity* newProjectile(Projectile* data, const char *filename);

void projectileDelete(Entity* self);

void projectileThink(Entity* self, float delta);
void projectileUpdate(Entity* self, float delta);

#endif