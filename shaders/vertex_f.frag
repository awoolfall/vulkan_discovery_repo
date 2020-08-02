#version 450
#extension GL_ARB_separate_shader_objects : enable

struct VS_OUT {
    vec3 color;
    vec2 texCoord;
    mat3 TBN;
};
layout(location = 0) in VS_OUT fs_in;

layout(set = 2, binding = 0) uniform sampler2D colorTex;
layout(set = 3, binding = 0) uniform sampler2D normTex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = mix(texture(colorTex, fs_in.texCoord), texture(normTex, fs_in.texCoord), 0.2);
}
