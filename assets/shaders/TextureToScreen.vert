#version 400

layout(location = 0) in vec2 pos;
layout(location = 2) in vec2 uv;


out vec2 vUV;
out vec3 vFragPos;

out vec3 vViewPos;
out mat3 vViewMatrix3x3;

//--------------uniform--------------/
layout (std140) uniform uCameraMat
{
	vec3 viewPos;
	float far;
	mat4 proj;
	mat4 view;
};

void main()
{
	gl_Position = vec4(pos * 2.0f, 0.0f, 1.0f);
	vFragPos = vec3(pos * 2.0f, 0.0f);
	vUV = uv;
	//vUV = (vFragPos.xy + vec2(1.0f))/2.0f;
	
	vViewPos = viewPos;
	vViewMatrix3x3 = mat3(view);
}

