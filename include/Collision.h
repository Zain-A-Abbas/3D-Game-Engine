#ifndef __COLLISIONS__
#define __COLLISIONS__

#include "gfc_vector.h"
#include "gfc_primitives.h"
#include "Extra_Primitives.h"

struct Entity_S;

void setCapsuleFinalRadius(GFC_Capsule* c, struct Entity_S* ent);

void setCapsuleFinalBase(GFC_Capsule* c, struct Entity_S* ent);

void setCapsuleFinalTip(GFC_Capsule* c, struct Entity_S* ent);

short capsuleToCapsuleTest(GFC_Capsule a, GFC_Capsule b, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth);

short capsuleToTriangleTest(GFC_Capsule c, GFC_Triangle3D t, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float* penetrationDepth);

short sphereTriangleTest(GFC_Sphere sphere, GFC_Triangle3D triangle, GFC_Vector3D* intersectionPoint, GFC_Vector3D* penetrationNormal, float *penetrationDepth);

GFC_Vector3D closestPointOnLineSegment(GFC_Vector3D a, GFC_Vector3D b, GFC_Vector3D point);

GFC_Box drawableBoxPrimitive(GFC_Box box);

#endif