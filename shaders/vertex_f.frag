#version 450
#extension GL_ARB_separate_shader_objects : enable

struct VS_OUT {
    vec3 color;
    vec2 texCoord;
    mat3 TBN;
    float currTime;
};
layout(location = 0) in VS_OUT fs_in;

layout(set = 2, binding = 0) uniform sampler2D colorTex;
layout(set = 3, binding = 0) uniform sampler2D normTex;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightColor = vec3(1.0);
    vec3 lightDir = normalize(vec3(2.0 * cos(fs_in.currTime), 2.0 * sin(fs_in.currTime), 0.0));

    float ambientStrength = 0.0;

    vec3 normal = texture(normTex, fs_in.texCoord).rgb;
    normal = (2.0 * normal) - 1.0;

    normal = normalize(fs_in.TBN * normal);

    vec3 diffuse = max(dot(normal, lightDir), ambientStrength) * lightColor;

    vec3 lightResult = diffuse;
    outColor = texture(colorTex, fs_in.texCoord);
    outColor = vec4(lightResult, 1.0) * outColor;
    // outColor = mix(vec4(fs_in.TBN[0].xyz, 1.0), vec4(fs_in.TBN[1].xyz, 1.0), 0.5);
    // outColor = vec4(normalize(texture(normTex, fs_in.texCoord).rgb), 1.0);
}
