#include "UI.h"
#include "Reticle.h"

const char* reticleActor = "actors/reticle.actor";
const char* uiBGActor = "actors/WeaponBG.actor";

UIData uiData = { 0 };

void initializeUI() {
	actorLoad(&uiData.bgActor, uiBGActor);

	actorLoad(&reticle.actor, reticleActor);
	reticle.hidden = false;

}

void actorLoad(Actor **actorptr, const char* actorFile) {
	if (*actorptr) gf2d_actor_free(*actorptr);
	*actorptr = gf2d_actor_load(actorFile);

	if (!*actorptr) {
		slog("Failed to load actor file %s into pointer", actorFile);
	}
}

void assignPlayer(PlayerData* playerData) {
	uiData.playerData = playerData;
}

void playerSwitchWeapon(Weapon* weapon) {
	actorLoad(&uiData.currentWeaponActor, weapon->actorFile);
}

void drawUI() {
	GFC_Vector2D resolution = gf3d_vgraphics_get_resolution();
	reticleDraw(resolution);
	drawPlayerUI(resolution);
}

void drawPlayerUI(GFC_Vector2D resolution) {
	if (!uiData.playerData) {
		return;
	}

	GFC_Vector2D uiPosition = gfc_vector2d_multiply(resolution, gfc_vector2d(0.2, 0.8));

	gf2d_actor_draw(
		uiData.bgActor,
		0,
		uiPosition,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (uiData.currentWeaponActor) {
		gf2d_actor_draw(
			uiData.currentWeaponActor,
			0,
			uiPosition,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		);
	}
}