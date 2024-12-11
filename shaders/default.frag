#version 450
#extension GL_ARB_separate_shader_objects : enable

const uint MAX_BONES = 128;
const uint MAX_SHADER_LIGHTS = 16;

struct MeshUBO
{
    mat4    model;
    mat4    view;
    mat4    proj;
    vec4    color;             
    vec4    camera;            //needed for many light calculations
};

struct ArmatureUBO
{
    mat4 bones[MAX_BONES];
};

struct MaterialUBO
{
    vec4    ambient;        //how much ambient light affects this material
    vec4    diffuse;        //how much diffuse light affects this material - primary influcen for material color
    vec4    specular;       //color of the shine on the materials
    vec4    emission;       //color that shows regardless of light
    float   transparency;   //how translucent the material should be overall
    float   shininess;      //how shiny the materials is.  // how pronounced the specular is
    vec2    padding;        //for alignment
};

struct Light {
    vec4    lightColor;
    vec4    lightPosition;
    vec4    lightDirection;
    float   angle;
    float   brightness;
    float   fallOff;
    float   lightActive;
};

struct LightUBO
{
    Light lights[MAX_SHADER_LIGHTS];   //list of all lights
};

layout(binding = 0) uniform UniformBufferObject
{
    MeshUBO         mesh;
    ArmatureUBO     armature;
    MaterialUBO     material;   //this may become an array
    LightUBO        lights;
    vec4            flags;      //.x is for bones
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 position;

layout(location = 0) out vec4 outColor;

void main()
{   
    int i;
    vec4 surfaceColor = texture(texSampler, fragTexCoord);
    vec3 normal = fragNormal;
    
    surfaceColor.xyz *= ubo.material.diffuse.xyz;
    surfaceColor.w *= ubo.material.diffuse.w * ubo.material.transparency;

    outColor = surfaceColor;

    vec4 compositeLight = vec4(0);

    for (int i = 0; i < MAX_SHADER_LIGHTS; i++) {
        if (ubo.lights.lights[i].lightActive != 0.0) {
            vec3 surfaceToCamera = normalize(ubo.mesh.camera.xyz - position);
            vec3 surfaceToLight = -normalize(ubo.lights.lights[i].lightDirection.xyz);

            if (ubo.lights.lights[i].lightPosition.w > 0.0) {
                surfaceToLight = normalize(ubo.lights.lights[i].lightPosition.xyz - position);
            }

            float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));

            vec3 diffuse = diffuseCoefficient * surfaceColor.rgb * ubo.lights.lights[i].lightColor.rgb * ubo.lights.lights[i].brightness;

            compositeLight += vec4(diffuse, 0);
        }
    }

    outColor.xyz += compositeLight.xyz;

}

