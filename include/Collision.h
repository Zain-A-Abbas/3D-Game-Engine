#ifndef __COLLISIONS__
#define __COLLISIONS__

#include "gfc_vector.h"
#include "gfc_primitives.h"
#include "Extra_Primitives.h"

struct Entity_S;
struct EntityCollision_S;

void setCapsuleFinalRadius(GFC_Capsule* c, struct Entity_S* ent);

void setCapsuleFinalBase(GFC_Capsule* c, struct Entity_S* ent);

void setCapsuleFinalTip(GFC_Capsule* c, struct Entity_S* ent);

short capsuleToCapsuleTest(GFC_Capsule a, GFC_Capsule b, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth);

short capsuleToTriangleTest(GFC_Capsule c, GFC_Triangle3D t, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth);

short sphereTriangleTest(GFC_Sphere sphere, GFC_Triangle3D triangle, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float *penetrationDepth);

GFC_Vector3D closestPointOnLineSegment(GFC_Vector3D a, GFC_Vector3D b, GFC_Vector3D point);

Uint8 edgeCapsuleTest(GFC_Edge3D e, GFC_Capsule c, GFC_Vector3D* poc, GFC_Vector3D* normal);
GFC_Box capsuleToBox(GFC_Capsule c);

GFC_Box drawableBoxPrimitive(GFC_Box box);

GFC_Vector3D cubeHalfDimensions(GFC_Box box);

void entityCollisionFree(struct Entity_S* self, struct EntityCollision_S* collision);

#endif