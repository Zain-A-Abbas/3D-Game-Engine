#include "light.h"
#include "simple_logger.h"

typedef struct {
	GFC_List	*lights;
} LightManager;

static LightManager lightManager = { 0 };

void initLights() {
	lightManager.lights = gfc_list_new();
	if (!lightManager.lights) {
		slog("Failed to allocate light manager.");
	}
	atexit(closeLightManager);
}

void closeLightManager() {
	for (int i = 0; i < gfc_list_count(lightManager.lights); ++i) {
		free(gfc_list_get_nth(lightManager.lights, i));
		gfc_list_delete_nth(lightManager.lights, i);
	}
	gfc_list_clear(lightManager.lights);
	memset(&lightManager, 0, sizeof(LightManager));
}

void freeLight(Light* light) {
	if (!light) {
		return;
	}
	gfc_list_delete_data(lightManager.lights, light);
	free(light);
}

void addDirectionalLight(GFC_Vector4D color, GFC_Vector4D direction, float brightness) {
	if (!lightManager.lights) {
		return;
	}
	Light* newLight = gfc_allocate_array(sizeof(Light), 1);
	if (!newLight) {
		return;
	}

	newLight->lightColor = color;
	newLight->lightDir = direction;
	newLight->lightPos = gfc_vector4d(0, 0, 0, 0);
	newLight->brightness = brightness;
	newLight->angle = 0.0;
	newLight->lightActive = 1.0;
	gfc_list_append(lightManager.lights, newLight);
}

Light* getLights() {
	return gfc_list_get_nth(lightManager.lights, 0);
}

LightUBO *gf3d_light_get_global_lights_ubo() {
	LightUBO *ubo = { 0 };
	ubo = gfc_allocate_array(sizeof(LightUBO), 1);
	gf3d_light_build_ubo_from_list(ubo, lightManager.lights);
	return ubo;
}

void gf3d_light_build_ubo_from_list(LightUBO* ubo, GFC_List* lights) {
	int i, c;
	Light* light;
	if ((!ubo) || (!lights))return;
	if (ubo->flags.x >= MAX_SHADER_LIGHTS)return;
	c = gfc_list_get_count(lights);
	for (i = 0; i < c; i++)
	{
		light = gfc_list_get_nth(lights, i);
		if (!light)continue;
		gf3d_light_add_light_to_ubo(ubo, light);
		if (ubo->flags.x >= MAX_SHADER_LIGHTS)return;
	}
}

void gf3d_light_add_light_to_ubo(LightUBO* ubo, Light* light) {
	if ((!ubo) || (!light))return;
	if (ubo->flags.x >= MAX_SHADER_LIGHTS)return;//can't fit any more lights
	memcpy(&ubo->lights[(Uint32)ubo->flags.x], light, sizeof(Light));
	ubo->flags.x++;
}