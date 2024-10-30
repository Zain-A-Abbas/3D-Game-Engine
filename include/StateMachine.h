#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include "gfc_list.h"

struct Entity_S;

typedef struct State_S {
    void (*enter)           (struct Entity_S *self);
    void (*exit)            (struct Entity_S *self);
    void (*think)           (struct Entity_S *self, float delta);
    void (*update)          (struct Entity_S *self, float delta);
    void                    *stateData;
} State;

typedef struct StateMachine_S
{
    int         stateCount;
    State       *stateList;
    State       *currentState;
} StateMachine;

/**
 * @brief Go to a new state
 */
void changeState(Entity_S * self, StateMachine stateMachine, State *oldState, State *newState);

#endif