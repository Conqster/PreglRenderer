#version 400

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 col;


layout (std140) uniform uCameraMat
{
	vec3 viewPos;
	float far;
	mat4 proj;
	mat4 view;
};


out VS_OUT
{
	vec3 fragPos;
	vec3 viewPos;
	vec3 normal;
	vec2 texCoord;
}vs_out;

uniform mat4 uModel;
uniform mat4 uLightSpaceMat;

void main()
{
	vec4 v_model_pos = uModel * vec4(pos, 1.0f);
	
	gl_Position = proj * view * v_model_pos;
	
	vs_out.fragPos = v_model_pos.xyz;
	vs_out.viewPos = viewPos;
	vs_out.normal = mat3(transpose(inverse(uModel))) * normalize(nor);
	vs_out.texCoord = uv;
}