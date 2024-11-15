#ifndef __QUADTREE__
#define __QUADTREE__

#include "gfc_list.h"
#include "gfc_primitives.h"
#include "gf3d_model.h"

struct Entity_S;

typedef struct QuadtreeNode_S {
	struct QuadtreeNode_S		*children[4];
	GFC_List					*triangleList; // Holds integer indexes of the triangles
	GFC_Box						AABB; 
} QuadtreeNode;

typedef struct {
	QuadtreeNode		*root;
	GFC_List			*leaves;
	GFC_Box				boundingBox;
} Quadtree;

Quadtree* newQuadTree(struct Entity_S *entity, GFC_Box boundingBox, int splitCount);
GFC_List* getTriangleIndexes(Quadtree* quadtree);
void addTrianglesToQuadtree(Quadtree* quadtree, struct Entity_S *ent, ObjData* obj);
void recursiveCreateNodeTree(Quadtree* quadtree, QuadtreeNode* node, GFC_Box parentAABB, int currentLevel, int splitCount);

GFC_Box triangleToBox(GFC_Triangle3D t);

void quadtreeFree(Quadtree *quadtree);
void quadtreeRecursiveFree(QuadtreeNode* node);

#endif