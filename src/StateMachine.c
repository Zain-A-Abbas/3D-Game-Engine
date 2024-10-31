#include "StateMachine.h"
#include "simple_logger.h"

State* createState(const char* name, void* enterFunction, void* exitFunction, void* thinkFunction, void* updateFunction) {
	State* newState = (State*)malloc(sizeof(State));
	if (!newState) {
		slog("Could not allocate new state");
		free(newState);
		return NULL;
	}

	memcpy(newState->stateName, name, sizeof(newState->stateName));
	newState->enter = enterFunction;
	newState->exit = exitFunction;
	newState->think = thinkFunction;
	newState->update = updateFunction;
	return newState;
}