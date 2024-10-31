#ifndef __ZOMBIE__
#define __ZOMBIE__

#include "Enemy.h"
#include "StateMachine.h"

Entity * createZombie();


void wanderThink(struct Entity_S* self, float delta);
void wanderUpdate(struct Entity_S* self, float delta);

#endif