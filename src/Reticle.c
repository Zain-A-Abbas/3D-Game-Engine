#include "Reticle.h"

Reticle reticle = {0};



void reticleDraw(GFC_Vector2D resolution) {
    if (reticle.hidden) return;
    GFC_Vector2D reticlePosition = gfc_vector2d(resolution.x / 2.0, resolution.y / 2.0);
    //slog("actorCenter: %f,%f",reticle.actor->center.x, reticle.actor->center.y);

    gf2d_actor_draw(
        reticle.actor,
        reticle.frame,
        reticlePosition,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

}