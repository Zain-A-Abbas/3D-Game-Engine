#include "UI.h"
#include "Reticle.h"
#include "gf2d_font.h"

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

void playerSwitchWeapon(Weapon weapon) {
	actorLoad(&uiData.currentWeaponActor, weapon.actorFile);
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
	if (uiData.playerData->weaponsUnlocked < 1) {
		return;
	}

	GFC_Vector2D weaponUiPosition = gfc_vector2d_multiply(resolution, gfc_vector2d(0.128, 0.8));
	GFC_Vector2D ammoTextPosition = gfc_vector2d(weaponUiPosition.x - 24, weaponUiPosition.y + 8);

	gf2d_actor_draw(
		uiData.bgActor,
		0,
		weaponUiPosition,
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
			weaponUiPosition,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		);

		Weapon currentWeaponData = uiData.playerData->playerWeapons[uiData.playerData->currentweapon];

		char* ammoText = malloc(8);
		snprintf(ammoText, 8, "%02d | %02d", currentWeaponData.currentAmmo, currentWeaponData.reserveAmmo);
		gf2d_font_draw_line_tag(ammoText, FT_Ammo, GFC_COLOR_WHITE, ammoTextPosition);

	}
}