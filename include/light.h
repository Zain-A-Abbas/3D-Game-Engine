#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "gfc_vector.h"
#include "gfc_list.h"

#define MAX_SHADER_LIGHTS   16

typedef struct
{
    /* data */
    GFC_Vector4D    lightColor;
    GFC_Vector4D    lightPos;
    GFC_Vector4D    lightDir;
    float           angle;
    float           brightness;
    float           falloff;
    float           lightActive;
} Light;

typedef struct
{
    Light lights[MAX_SHADER_LIGHTS];   //list of all lights
    GFC_Vector4D    flags;    // x is how many lights are in use
}LightUBO;

void initLights();
void closeLightManager();
void addDirectionalLight(GFC_Vector4D color, GFC_Vector4D direction, float brightness);
LightUBO *gf3d_light_get_global_lights_ubo();
void gf3d_light_build_ubo_from_list(LightUBO* ubo, GFC_List* lights);
void gf3d_light_add_light_to_ubo(LightUBO* ubo, Light* light);

#endif