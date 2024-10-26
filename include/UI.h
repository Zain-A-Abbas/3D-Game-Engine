#ifndef __UI__
#define __UI__

#include "simple_logger.h"
#include "gf2d_actor.h"
#include "Player.h"
#include "Weapon.h"

typedef struct {
	PlayerData*		playerData;

} UIData ;

void initializeUI();

void assignPlayer(PlayerData* playerData);

void drawUI();

void drawPlayerUI();

#endif