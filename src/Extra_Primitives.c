#include "Extra_Primitives.h"
#include "Collision.h"

GFC_Capsule gfc_capsule(float height, float radius) {
	GFC_Capsule c = { 0 };
	c.height = height;
	c.radius = radius;
	c.tip = gfc_vector3d(0, 0, height);
	c.finalBase = gfc_vector3d(0, 0, 0);
	c.finalTip = c.tip;
	return c;
}

void drawBoundingBox(GFC_Box box, GFC_Color color, Uint8 insideOut) {
	GFC_Vector3D scale = gfc_vector3d(0.5, 0.5, 0.5);
	if (insideOut) {
		scale.x *= -1;
		scale.y *= -1;
		scale.z *= -1;
	}
	gf3d_draw_cube_solid(
		box,
		cubeHalfDimensions(box),
		gfc_vector3d(0, 0, 0),
		scale,
		color
	);
}
