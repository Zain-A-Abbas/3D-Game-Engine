#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include "gfc_list.h"
#include "Entity.h"

typedef struct StateMachine_S StateMachine;
typedef struct State_S State;

typedef struct State_S {
    char                    stateName[32];
    void (*enter)           (struct Entity_S *self, struct State_S *state, StateMachine *stateMachine);
    void (*exit)            (struct Entity_S *self, struct State_S* state, StateMachine* stateMachine);
    void (*think)           (struct Entity_S *self, float delta, struct State_S* state, StateMachine* stateMachine);
    void (*update)          (struct Entity_S *self, float delta, struct State_S* state, StateMachine* stateMachine);
    void                    *stateData;
    StateMachine            *stateMachine;
} State;

typedef struct StateMachine_S
{
    GFC_List    *stateList;
    State       *currentState;
} StateMachine;

/**
 * @brief Go to a new state
 */
void changeState(Entity * self, StateMachine *stateMachine, char newState[32]);

/**
* @Creates a state of a certain name and adds it to the given state machine
*/
State* createState(const char* name, StateMachine *stateMachine, void* enterFunction, void* exitFunction, void* thinkFunction, void* updateFunction, void* stateDate);

#endif