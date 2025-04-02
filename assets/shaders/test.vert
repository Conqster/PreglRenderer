#version 400

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 col;

out vec3 v_Colour;
out vec2 v_UV;
out vec3 vNormal;
out vec4 vFragPosLightSpace;

uniform mat4 uModel;
uniform mat4 uproj;
uniform mat4 uview;
uniform mat4 uLightSpaceMat;

void main()
{
	gl_Position = uproj * uview * uModel * vec4(pos, 1.0f);
	v_Colour = col;
	v_UV = uv;
	vNormal = mat3(transpose(inverse(uModel))) * normalize(nor);
	//vNormal = mat3(transpose((uModel)))* normalize(nor);
	
	vFragPosLightSpace = uLightSpaceMat * uModel * vec4(pos, 1.0f);
}
