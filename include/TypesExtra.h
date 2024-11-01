#ifndef __TYPES_EXTRA__
#define __TYPES_EXTRA__
#include "gfc_types.h"
#include "gfc_vector.h"

/**
* @brief Moves a float towards a certain value by delta amount.
*/
float fMoveTowards(float current, float target, float delta);
float fMoveTowardsAngle(float current, float target, float delta);
float gfc_vector2d_cross_product(GFC_Vector2D v1, GFC_Vector2D v2);

#endif