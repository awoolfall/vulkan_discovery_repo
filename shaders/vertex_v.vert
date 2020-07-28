#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform ModelData {
    mat4 transform;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    /* convert to vulkan coordinates (+y down) */
    vec4 vertPos = vec4(inPosition, 1.0);
    vertPos.y = -vertPos.y;

    gl_Position = (ubo.proj * ubo.view * model.transform) * vertPos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
