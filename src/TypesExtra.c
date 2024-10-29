#include "TypesExtra.h"

float fMoveTowards(float current, float target, float delta) {
    if (current < target) {
        return fminf(current + delta, target);
    }
    else if (current > target) {
        return fmaxf(current - delta, target);
    }
}