#include "Quadtree.h"
#include "simple_logger.h"

Quadtree* newQuadTree(Entity* entity, GFC_Box boundingBox, int splitCount) {
	if (!entity->entityCollision) {
		slog("No entitycollision to insert quadtree into.");
		return NULL;
	}
	if (entity->entityCollision->collisionPrimitive) {
		slog("Entity already has primitive");
	}
	if (entity->entityCollision->quadTree) {
		slog("Quadtree already exists for entity.");
		return NULL;
	}

	Quadtree* quadtree = (Quadtree*) malloc(sizeof(Quadtree));
	if (!quadtree) {
		slog("Could not allocate quadtree");
	}
	memset(quadtree, 0, sizeof(quadtree));
	entity->entityCollision->AABB = boundingBox;

	QuadtreeNode *root = (QuadtreeNode*) malloc(sizeof(QuadtreeNode));
	memset(root, 0, sizeof(QuadtreeNode));
	root->AABB = boundingBox;
	
	quadtree->root = root;
	recursiveCreateNodeTree(root, boundingBox, 0, splitCount);

}

void recursiveCreateNodeTree(QuadtreeNode* node, GFC_Box parentAABB, int currentLevel, int splitCount) {
	if (currentLevel == splitCount) {
		return;
	}
	//node->children = (QuadtreeNode*) malloc(4 * sizeof(QuadtreeNode));
	for (int i = 0; i < 4; i++) {
		QuadtreeNode *newNode = (QuadtreeNode*) malloc(sizeof(QuadtreeNode));
		memset(newNode, 0, sizeof(QuadtreeNode));
		node->children[i] = newNode;
		GFC_Box childAABB = { 0 };
		childAABB.x = parentAABB.x;
		childAABB.y = parentAABB.y;
		childAABB.z = parentAABB.z;
		childAABB.w = parentAABB.w / 2;
		childAABB.d = parentAABB.d / 2;
		childAABB.h = parentAABB.h;
		if (i == 1 || i == 3) {
			childAABB.x += parentAABB.w / 2;
		}
		if (i == 2 || i == 3) {
			childAABB.y += parentAABB.d / 2;
		}

		newNode->AABB = childAABB;
		recursiveCreateNodeTree(&newNode, childAABB, currentLevel + 1, splitCount);
	}
}