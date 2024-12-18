#include "UI.h"
#include "Reticle.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "Level.h"

const char* reticleActor = "actors/reticle.actor";
const char* uiBGActor = "actors/WeaponBG.actor";
const char* hpBorderActor = "actors/HPBorder.actor";
const char* hpBarActor = "actors/HPBar.actor";
const char* moneyActor = "actors/Money.actor";

const GFC_Rect TEXT_RECT = {200, 200, 200, 200};
const GFC_Vector2D ENEMY_KILLS_LOCATION = { 450, 50 };
const GFC_Vector2D ENEMY_KILLS_DROP_SHADOW_LOCATION = { 452, 52 };

UIData uiData = { 0 };

void initializeUI() {
	actorLoad(&uiData.bgActor, uiBGActor);
	actorLoad(&reticle.actor, reticleActor);
	actorLoad(&uiData.hpBorderActor, hpBorderActor);
	actorLoad(&uiData.hpBarActor, hpBarActor);
	actorLoad(&uiData.moneyActor, moneyActor);
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

void playerSwitchWeapon(Weapon *weapon) {
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
	if (uiData.playerData->weaponsUnlocked < 1) {
		return;
	}

	if (uiData.playerData->currentInteractable) {
		if (strlen(uiData.playerData->currentInteractable->interactText) > 0) {
			GFC_Vector2D textPosition = gfc_vector2d(resolution.x / 2 - 20 * strlen(uiData.playerData->currentInteractable->interactText), resolution.y / 2 + 20);
			gf2d_font_draw_line_tag(
				uiData.playerData->currentInteractable->interactText,
				FT_Interactable,
				GFC_COLOR_BLACK,
				gfc_vector2d(textPosition.x + 2, textPosition.y + 2)
			);
			gf2d_font_draw_line_tag(
				uiData.playerData->currentInteractable->interactText,
				FT_Interactable,
				GFC_COLOR_WHITE,
				textPosition
			);
		}
	}

	// Weapons
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

		Weapon* currentWeaponData = (Weapon*)gfc_list_get_nth(uiData.playerData->playerWeapons, uiData.playerData->currentWeapon);//uiData.playerData->playerWeapons[uiData.playerData->currentWeapon];

		char* ammoText = malloc(8);
		snprintf(ammoText, 8, "%02d | %02d", currentWeaponData->currentAmmo, uiData.playerData->ammo[currentWeaponData->reserveAmmoIndex]);
		gf2d_font_draw_line_tag(ammoText, FT_Ammo, GFC_COLOR_WHITE, ammoTextPosition);

	}

	// HP
	GFC_Vector2D hpUIPosition = weaponUiPosition;
	hpUIPosition.y += 80;
	gf2d_actor_draw(
		uiData.hpBorderActor,
		0,
		hpUIPosition,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);
	GFC_Vector2D hpBarPosition = hpUIPosition;
	hpBarPosition.y += 2;
	int i = 0;
	for (i = 0; i < 128 * ((float)uiData.playerData->hp / 100); i++) {
		hpBarPosition.x = hpUIPosition.x + (i + 2);
		gf2d_actor_draw(
			uiData.hpBarActor,
			0,
			hpBarPosition,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		);
	}

	// Money

	GFC_Vector2D moneyUIPosition = hpUIPosition;
	moneyUIPosition.x += 96;
	GFC_Rect moneyRect = gfc_rect(moneyUIPosition.x - 26, moneyUIPosition.y - 32, 112, 60);
	gf2d_draw_rect_filled(
		moneyRect,
		gfc_color(0.0, 0.0, 0.0, 0.5)
	);

	
	gf2d_actor_draw(
		uiData.moneyActor,
		0,
		moneyUIPosition,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	GFC_Vector2D moneyTextPosition = moneyUIPosition; moneyTextPosition.x += 28; moneyTextPosition.y -= 8;
	char* money[4];
	sprintf(money, "%d", uiData.playerData->money);
	gf2d_font_draw_line_tag(money, FT_Ammo, GFC_COLOR_WHITE, moneyTextPosition);

	// Enemy kills/Boss hp
	drawEnemyKills();

}

void drawEnemyKills() {
	if (!levelData) {
		return;
	}

	char* enemyKills[25];
	sprintf(enemyKills, "Enemy Kill Count: %d/%d", levelData->enemyKillCount, levelData->enemyTargetCount);
	gf2d_font_draw_line_tag(enemyKills, FT_Large, GFC_COLOR_BLACK, ENEMY_KILLS_DROP_SHADOW_LOCATION);
	gf2d_font_draw_line_tag(enemyKills, FT_Large, GFC_COLOR_WHITE, ENEMY_KILLS_LOCATION);
}