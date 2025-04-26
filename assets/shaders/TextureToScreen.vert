#version 400

layout(location = 0) in vec2 pos;
layout(location = 2) in vec2 uv;


out vec2 vUV;
out vec3 vFragPos;
void main()
{
	gl_Position = vec4(pos * 2.0f, 0.0f, 1.0f);
	vUV = uv;
	vFragPos = vec3(pos * 2.0f, 0.0f);
}

