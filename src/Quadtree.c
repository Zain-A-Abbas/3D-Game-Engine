#include "Quadtree.h"
#include "simple_logger.h"
#include "gf3d_obj_load.h"
#include "gfc_primitives.h"

Quadtree* newQuadTree(Entity* entity, GFC_Box boundingBox, int splitCount) {
	printf("\nStart");
	if (splitCount < 1) {
		slog("Split count must be minimum 1");
		return NULL;
	}
	if (!entity->model) {
		slog("No model to search for triangles.");
		return NULL;
	}
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

	Quadtree* quadtree = (Quadtree*)malloc(sizeof(Quadtree));
	if (!quadtree) {
		slog("Could not allocate quadtree");
	}
	memset(quadtree, 0, sizeof(quadtree));


	GFC_List* leaves = gfc_list_new_size(4 * splitCount);
	if (!leaves) {
		slog("Could not create leaves list.");
		free(quadtree);
		return NULL;
	}

	quadtree->leaves = leaves;

	entity->entityCollision->AABB = boundingBox;

	QuadtreeNode* root = (QuadtreeNode*)malloc(sizeof(QuadtreeNode));
	memset(root, 0, sizeof(QuadtreeNode));
	root->AABB = boundingBox;

	quadtree->root = root;
	recursiveCreateNodeTree(quadtree, root, boundingBox, 0, splitCount);

	for (int i = 0; i < gfc_list_get_count(entity->model->mesh_list); i++) {
		Mesh* mesh = (Mesh*)gfc_list_get_nth(entity->model->mesh_list, i);
		if (mesh) {
			// Get primitives
			for (int j = 0; j < gfc_list_get_count(mesh->primitives); j++) {
				MeshPrimitive* primitive = (MeshPrimitive*)gfc_list_get_nth(mesh->primitives, j);
				if (primitive) {
					if (primitive->objData) {
						addTrianglesToQuadtree(quadtree, entity, primitive->objData);
					}
				}
			}
		}
	}
	entity->entityCollision->AABB = boundingBox;
	entity->entityCollision->quadTree = quadtree;
	printf("\nFinish\n");
}

GFC_List* getTriangleIndexes(Quadtree* quadtree) {
	return quadtree->leaves;
}

void addTrianglesToQuadtree(Quadtree* quadtree, Entity* ent, ObjData* obj) {
	int i;
	Uint32 index;
	GFC_Triangle3D t = { 0 };
	GFC_Vector3D entityPosition = entityGlobalPosition(ent);
	GFC_Vector3D entityRotation = entityGlobalRotation(ent);
	GFC_Vector3D entityScale = entityGlobalScale(ent);


	for (i = 0; i < obj->face_count; i++) {
		index = obj->outFace[i].verts[0];
		t.a = obj->faceVertices[index].vertex;
		index = obj->outFace[i].verts[1];
		t.b = obj->faceVertices[index].vertex;
		index = obj->outFace[i].verts[2];
		t.c = obj->faceVertices[index].vertex;

		t.a = gfc_vector3d_multiply(t.a, entityScale);
		t.b = gfc_vector3d_multiply(t.b, entityScale);
		t.c = gfc_vector3d_multiply(t.c, entityScale);

		gfc_vector3d_rotate_about_z(&t.a, entityRotation.z);
		gfc_vector3d_rotate_about_z(&t.b, entityRotation.z);
		gfc_vector3d_rotate_about_z(&t.c, entityRotation.z);

		t.a = gfc_vector3d_added(t.a, entityPosition);
		t.b = gfc_vector3d_added(t.b, entityPosition);
		t.c = gfc_vector3d_added(t.c, entityPosition);

		GFC_Vector3D currentPoint = t.a;

		GFC_Box triangleBox = triangleToBox(t);
		for (int j = 0; j < gfc_list_get_count(quadtree->leaves); j++) {
			QuadtreeNode *node = (QuadtreeNode*)gfc_list_get_nth(quadtree->leaves, j);
			if (gfc_box_overlap(triangleBox, node->AABB)) {
				int* triangleIndex = (int*)malloc(sizeof(int));
				*triangleIndex = i;
				gfc_list_append(node->triangleList, triangleIndex);
			}
		}
	}
}

void recursiveCreateNodeTree(Quadtree* quadtree, QuadtreeNode* node, GFC_Box parentAABB, int currentLevel, int splitCount) {
	if (currentLevel == splitCount) {
		//printf("\nCurrent level: %d", currentLevel);
		//printf("\nQuadtree leaves: %d", gfc_list_get_count(quadtree->leaves));
		gfc_list_append(quadtree->leaves, node);
		node->triangleList = gfc_list_new_size(8);
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
		childAABB.d = parentAABB.d;
		childAABB.h = parentAABB.h / 2;
		if (i == 1 || i == 3) {
			childAABB.x += parentAABB.w / 2;
		}
		if (i == 2 || i == 3) {
			childAABB.y += parentAABB.h / 2;
		}

		newNode->AABB = childAABB;
		recursiveCreateNodeTree(quadtree, newNode, childAABB, currentLevel + 1, splitCount);
	}
}

GFC_Box triangleToBox(GFC_Triangle3D t) {
	GFC_Box b;
	b.x = MIN(MIN(t.a.x, t.b.x), t.c.x);
	b.y = MIN(MIN(t.a.y, t.b.y), t.c.y);
	b.z = MIN(MIN(t.a.z, t.b.z), t.c.z);
	b.w = MAX(MAX(t.a.x, t.b.x), t.c.x) - b.x;
	b.h = MAX(MAX(t.a.y, t.b.y), t.c.y) - b.y;
	b.d = MAX(MAX(t.a.z, t.b.z), t.c.z) - b.z;
	return b;
}

