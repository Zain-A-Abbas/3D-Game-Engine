#include "UI.h"
#include "Reticle.h"

const char* reticleActor = "actors/reticle.actor";


UIData uiData = { 0 };

void initializeUI() {
	reticleLoad(reticleActor);
}

void assignPlayer(PlayerData* playerData) {
	uiData.playerData = playerData;
}

void drawUI() {
	reticleDraw();
	drawPlayerUI();
}

void drawPlayerUI() {
	if (!uiData.playerData) {
		return;
	}
	slog("HOOrayyy");
}