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
    float currTime;
};
layout(location = 0) out VS_OUT vs_out;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    float currTime;
} ubo;

layout(set = 1, binding = 0) uniform ModelData {
    mat4 transform;
} model;

void main() {
    gl_Position = (ubo.proj * ubo.view * model.transform) * vec4(inPosition, 1.0);
    vs_out.color = inColor;
    vs_out.texCoord = inTexCoord;

    /* normal mapping */

    vec3 normal = normalize(inNormal);
    vec3 tangent = normalize(inTangent.xyz);

    mat3 normMat = transpose(inverse(mat3(model.transform)));
    normal = normalize(vec3(normMat * normal));
    tangent = normalize(vec3(normMat * tangent));

    // tangent = normalize(tangent - dot(tangent, normal) * normal);

    vec3 bitangent = normalize(cross(normal, tangent) * inTangent.w);

    // /* tangent handedness fix */
    // if (dot(cross(tangent, bitangent), normal) < 0.0) {
    //     tangent = -tangent;
    // }

    vs_out.TBN = mat3(tangent, bitangent, normal);

    vs_out.currTime = ubo.currTime;
}
