#version 420 core

uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;
uniform mat4 u_ModelViewProjection;
uniform float t;

//Keyframe 0 vertex position.
layout(location = 0) in vec3 inPos_0;
//Keyframe 1 vertex position.
layout(location = 1) in vec3 inPos_1;
//Keyframe 0 vertex normal.
layout(location = 2) in vec3 inNorm_0;
//Keyframe 1 vertex normal.
layout(location = 3) in vec3 inNorm_1;

//The vertex position and normal we will send to
//the fragment shader.
layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;

void main()
{
    
    //Output position - our viewprojection matrix
    //multiplied by world-space position.
    gl_Position = u_ModelViewProjection * vec4(mix(inPos_0, inPos_1, t), 1.0);

    //World-space vertex - LERP the verts!
    //(The final LERPed vert is, as always, transformed
    //by the model matrix.)
    outWorldPos = (u_Model * vec4(mix(inPos_0, inPos_1, t), 1.0)).xyz;

    //World-space normal - LERP the normals!
    //(And transform by the normal matrix.)
    outNormal = u_NormalMatrix * mix(inNorm_0, inNorm_1, t);
    
}

