#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in mat3 fragTBN;

layout(set = 2, binding = 0) uniform sampler2D colorTex;
layout(set = 3, binding = 0) uniform sampler2D normTex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = mix(texture(colorTex, fragTexCoord), texture(normTex, fragTexCoord), 0.2);
}
