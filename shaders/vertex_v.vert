#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out mat3 fragTBN;

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
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragTBN = mat3(0.0);
}
