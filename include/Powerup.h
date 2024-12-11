#ifndef __POWERUP__
#define __POWERUP__

#include "Entity.h"

typedef enum {
    INFINITE_AMMO,
    NO_RECOIL,
    QUARTER_RELOAD_TIME,
    INVINCIBLE,
    SPEED_BOOST
}PowerupType;

typedef struct {
    PowerupType     type;
    Entity          *player;
}PowerupData;

Entity *powerupEntityNew();

void *powerupThink(struct Entity_S *self, float delta);
void *powerupUpdate(struct Entity_S *self, float delta);
void *powerupDraw(Entity * self);

#endif