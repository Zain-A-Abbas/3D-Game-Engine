#include "StateMachine.h"
#include "simple_logger.h"

State* createState(const char* name, StateMachine *stateMachine, void* enterFunction, void* exitFunction, void* thinkFunction, void* updateFunction, void* onHitFunction, void *stateData) {
	State* newState = (State*)malloc(sizeof(State));
	if (!newState) {
		slog("Could not allocate new state");
		free(newState);
		return NULL;
	}

	if (!stateMachine->stateList) {
		stateMachine->stateList = gfc_list_new();
	}

	memcpy(newState->stateName, name, sizeof(newState->stateName));
	newState->enter = enterFunction;
	newState->exit = exitFunction;
	newState->think = thinkFunction;
	newState->update = updateFunction;
	newState->onHit = onHitFunction;
	newState->stateMachine = stateMachine;
	newState->stateData = stateData;
	gfc_list_append(stateMachine->stateList, newState);
	return newState;
}

void changeState(Entity* self, StateMachine *stateMachine, char newStateName[32]) {
	if (stateMachine->currentState) {
		if (stateMachine->currentState->exit) {
			stateMachine->currentState->exit(self, stateMachine->currentState, stateMachine);
		}
	}

	int i = 0;
	State* newState = NULL;
	State* currentState;
	for (i = 0; i < stateMachine->stateList->count; ++i) {
		currentState = (State*)gfc_list_get_nth(stateMachine->stateList, i);
		if (strcmp(currentState->stateName, newStateName) == 0) {
			newState = currentState;
		}
	}
	if (!newState) {
		slog("State %s could not be found.", newStateName);
		return;
	}

	stateMachine->currentState = newState;
	if (newState->enter) {
		newState->enter(self, newState, stateMachine);
	}
	
}

void stateMachineFree(StateMachine* stateMachine) {
	int i;
	State* currentState;
	for (i = 0; i < stateMachine->stateList->count; ++i) {
		currentState = (State*)gfc_list_get_nth(stateMachine->stateList, i);
		if (currentState->stateData) {
			free(currentState->stateData);
		}
		free(currentState); 
	}
	free(stateMachine);
}