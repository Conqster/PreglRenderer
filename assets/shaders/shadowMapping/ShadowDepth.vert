#version 400

layout (location = 0) in vec4 pos;

uniform mat4 uLightSpaceMat;
uniform mat4 uModel;

void main()
{
    gl_Position = uLightSpaceMat * uModel * pos;   
}