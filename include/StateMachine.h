#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include "gfc_list.h"
#include "Entity.h"


typedef struct State_S {
    char                    stateName[32];
    void (*enter)           (struct Entity_S *self);
    void (*exit)            (struct Entity_S *self);
    void (*think)           (struct Entity_S *self, float delta);
    void (*update)          (struct Entity_S *self, float delta);
    void                    *stateData;
} State;

typedef struct StateMachine_S
{
    GFC_List    *stateList;
    State       *currentState;
} StateMachine;

/**
 * @brief Go to a new state
 */
void changeState(Entity * self, StateMachine stateMachine, State *newState);

State* createState(const char* name, void* enterFunction, void* exitFunction, void* thinkFunction, void* updateFunction);

#endif