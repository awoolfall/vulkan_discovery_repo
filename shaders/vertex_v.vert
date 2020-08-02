#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;

struct VS_OUT {
    vec3 color;
    vec2 texCoord;
    mat3 TBN;
};
layout(location = 0) out VS_OUT vs_out;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

layout(set = 1, binding = 0) uniform ModelData {
    mat4 transform;
} model;

void main() {
    /* convert to vulkan coordinates (+y down) */
    vec4 vertPos = vec4(inPosition, 1.0);
    vertPos.y = -vertPos.y;

    gl_Position = (ubo.proj * ubo.view * model.transform) * vertPos;
    vs_out.color = inColor;
    vs_out.texCoord = inTexCoord;

    // normals (with re-orthoganalising)
    vec3 tangent = normalize(vec3(model.transform * vec4(inTangent)));
    vec3 normal = normalize(vec3(model.transform * vec4(inNormal, 0.0)));
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(normal, tangent);
    vs_out.TBN = mat3(tangent, bitangent, normal);
}
