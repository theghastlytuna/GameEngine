#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_uncommon.glsl"

uniform float t;

void main() {

	// Lecture 5
	// Pass vertex pos in world space to frag shader
	outWorldPos = (u_Model * mix(vec4(inPosition, 1.0), vec4(inPosition2, 1.0), t)).xyz;

	// Normals
	outNormal = mat3(u_NormalMatrix) * mix(inNormal, inNormal2, t);

	// Pass our UV coords to the fragment shader
	outUV = inUV;

	///////////
	outColor = inColor;

	gl_Position = u_ModelViewProjection *  mix(vec4(inPosition, 1.0), vec4(inPosition2, 1.0), t);

}

