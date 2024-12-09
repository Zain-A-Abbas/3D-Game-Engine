#include "Powerup.h"
#include "simple_logger.h"

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
    powerupData->type = gfc_random_int(4);
    powerupData->player = player;
    newPowerup->data = powerupData;

    newPowerup->update = powerupUpdate;
    newPowerup->think = powerupThink;

    

}

void *powerupThink(struct Entity_S *self, float delta) {
    PowerupData *powerupData = (PowerupData*)self->data;
    if (fabsf(self->position.x - powerupData->player->position.x) > 8 || fabsf(self->position.y - powerupData->player->position.y) > 8) {
        printf("\nToo far");
        return NULL;
    }
    printf("\nNot far");
}

void *powerupUpdate(struct Entity_S *self, float delta) {
    self->rotation.z += 1 * delta;
}