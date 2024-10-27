#ifndef __STRUCTURE__
#define __STRUCTURE__

#include "Entity.h"

typedef enum {
	HOUSE
} StructureType;

typedef struct StructureData_S {
	StructureType		structureType;
} StructureData;

Entity* structureNew(StructureType type);

#endif