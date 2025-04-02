#version 400

layout (location = 0) in vec3 pos;		
layout (location = 1) in vec4 colour; 	


uniform mat4 uProj;
uniform mat4 uView;

//later could have start & end colour;
out vec4 o_Colour;

void main()
{
	gl_Position = uProj * uView * vec4(pos, 1.0f);
	o_Colour = colour;
}