#include "Extra_Primitives.h"

GFC_Capsule gfc_capsule(float height, float radius) {
	GFC_Capsule c = { 0 };
	c.height = height;
	c.radius = radius;
	c.tip = gfc_vector3d(0, 0, height);
	c.finalBase = gfc_vector3d(0, 0, 0);
	c.finalTip = c.tip;
	return c;
}