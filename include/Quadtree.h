#ifndef __QUADTREE__
#define __QUADTREE__

#include "gfc_list.h"
#include "gfc_primitives.h"
#include "gf3d_model.h"
#include "Entity.h"

typedef struct QuadtreeNode_S {
	struct QuadtreeNode_S		*children[4];
	Bool						activeChildren[4];	// Which children are nodes and which are empty
	GFC_List					*triangleList; // Holds integer indexes of the triangles
	GFC_Box						AABB; 
} QuadtreeNode;

typedef struct {
	QuadtreeNode		*root;
	GFC_Box				boundingBox;
} Quadtree;

Quadtree* newQuadTree(Entity *entity, GFC_Box boundingBox, int splitCount);

#endif