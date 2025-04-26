#version 400

out vec4 FragColour;

in vec2 vUV;
in vec3 vFragPos;
uniform sampler2D uTexImage;

float blend(float v, float inner, float outer)
{
	float result = step(inner, v) * mix(0.0f, outer, v);
	result += step(v, inner) * mix(outer, 0.0f, v);
	return result;
}

void main()
{
	FragColour = vec4(texture(uTexImage, vUV).rgb, 1.0f);
	//FragColour.rg = vUV;
	//FragColour.rgb = vFragPos;
	//FragColour.rgb = abs(vFragPos);
	//FragColour = vec4(0.0f, 0.0f, 1.0f, 1.0f) * vUV.r;
	
	
	float v = mix(vUV.r, vUV.g , 1.0f);
	v = mix(0.0f, 0.5f, vUV.r);
	
	//v += mix(0.6f, 0.4f, vUV.r);
	float gradient =  vUV.r;
	
	//[0 > 1] --> [0.5 > 1]
	//float h = mix(vUV.r, 2.0f * (vUV.r - 0.5f), step(0.5f, vUV.r));
	
	float inner = 0.5f;
	float outer = 0.7f;
	float h = step(inner, vUV.r);
	h *= mix(0.0f, outer, vUV.r);
	h += step(vUV.r, inner) * mix(outer, 0.0f, vUV.r);
	
	v = step(inner, vUV.g);
	v *= mix(0.0f, 0.7f, vUV.g);
	v += step(vUV.g, inner) * mix(outer, 0.0f, vUV.g);
	
	v = blend(vUV.g, inner, outer);
	h = blend(vUV.r, inner, outer);
	gradient = max(v, h);
	gradient = v + h;
	vec4 out_colour = vec4(abs(vFragPos), 1.0f);
	out_colour.a = 0.0f;
	FragColour = out_colour * gradient;
}