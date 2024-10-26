#ifndef __UI__
#define __UI__

#include "simple_logger.h"
#include "gf2d_actor.h"
#include "Player.h"
#include "Weapon.h"

typedef struct {
	PlayerData*		playerData;
	Actor*			bgActor;
} UIData ;

void initializeUI();

void actorLoad(Actor* actorptr, const char* actorFile);

void assignPlayer(PlayerData* playerData);

void drawUI();

void drawPlayerUI(GFC_Vector2D resolution);

#endif