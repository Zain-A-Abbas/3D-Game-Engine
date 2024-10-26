#ifndef __RETICLE__
#define __RETICLE__

#include "simple_logger.h"
#include "gf2d_actor.h"
#include "gf3d_vgraphics.h"
#include <stdbool.h>    

typedef struct {
    Actor   *actor;
    float   frame;
    bool    hidden;
} Reticle;


extern Reticle reticle;



/**
 * @brief Make the reticle visible
 */
void reticleShow();

/**
 * @brief Make the reticle invisible
 */
void reticleHide();

/**
 * @brief Check if the reticle is visible or not
 */
int reticleIsHidden();

/**
 * @brief Called once per frame to draw the reticle
 */
void reticleDraw();

void reticleAnimate();

#endif