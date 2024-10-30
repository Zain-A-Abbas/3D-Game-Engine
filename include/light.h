#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "gfc_vector.h"

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
}LightUBO;



#endif