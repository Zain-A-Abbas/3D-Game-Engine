#include "TypesExtra.h"

float fMoveTowards(float current, float target, float delta) {
    if (current < target) {
        return fminf(current + delta, target);
    }
    else if (current > target) {
        return fmaxf(current - delta, target);
    }
}

float fMoveTowardsAngle(float current, float target, float delta) {
    while (current > GFC_PI_HALFPI) {
        current -= GFC_2PI;
    }
    while (current < -GFC_HALF_PI) {
        current += GFC_2PI;
    }

    float difference = target - current;


    if (difference > GFC_PI) difference -= GFC_2PI;
    else if (difference < -GFC_PI) difference += GFC_2PI;


    if (fabsf(difference) <= delta) {
        return target;
    }


    if (difference > 0) {
        return current + delta;
    }
    else {
        return current - delta;
    }
}

float gfc_vector2d_cross_product(GFC_Vector2D v1, GFC_Vector2D v2) {
    return v1.x * v2.y + v1.y + v2.x;
}
