#include "Powerup.h"
#include "simple_logger.h"
#include "Player.h"

const float powerupDuration = 8;

Entity *powerupEntityNew(Entity *player) {
    Entity *newPowerup = entityNew();
    if (!newPowerup) {
        slog("Not enough memory to create Powerup entity.");
        return NULL;
    }

    PowerupData *powerupData = (PowerupData*)malloc(sizeof(PowerupData));
    if (!powerupData) {
        free(newPowerup);
        slog("Not enough memory to create Powerup data.");
        return NULL;
    }
    memset(powerupData, 0, sizeof(powerupData));
    powerupData->type = gfc_random_int(SPEED_BOOST);
    powerupData->player = player;
    newPowerup->data = powerupData;

    newPowerup->update = powerupUpdate;
    newPowerup->think = powerupThink;

    

}

void *powerupThink(struct Entity_S *self, float delta) {
    PowerupData *powerupData = (PowerupData*)self->data;
    if (fabsf(self->position.x - powerupData->player->position.x) <= 4 && fabsf(self->position.y - powerupData->player->position.y) <= 4) {
        PlayerData* playerData = (PlayerData*) powerupData->player->data;
        playerData->powerups[powerupData->type] = 1;
        playerData->powerupTimers[powerupData->type] = powerupDuration;
        free(powerupData);
        _entityFree(self);

    }
}

void *powerupUpdate(struct Entity_S *self, float delta) {
    self->rotation.z += 1 * delta;
}