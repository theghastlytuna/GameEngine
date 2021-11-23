#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

void main() {
	vec3 vert = inPosition;
	vert.y = sin(vert.x * 5.0 + u_Time) * 0.2;

	outWorldPos = (u_Model * vec4(vert, 1.0)).xyz;

    // Project the world position to determine the screenspace position
	gl_Position = u_ViewProjection * vec4(outWorldPos, 1);

	// Normals
	outNormal = mat3(u_NormalMatrix) * inNormal;
	// Pass our UV coords to the fragment shader
	outUV = inUV;
	outColor = inColor;
}

