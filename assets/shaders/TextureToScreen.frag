#version 400

out vec4 FragColour;

in vec2 vUV;
in vec3 vFragPos;
uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uNoiseTex;

uniform vec3 uSamples[64];
uniform mat4 uProjection;

int kernelSize = 64;
float radius = 0.5f;
float bias = 0.025f;

const vec2 noiseScale = vec2(800.0f/4.0f, 600.0f/4.0f);

void main()
{

	float offset = 1.0f / 1000.0f;
	
	vec2 offsets[9] = vec2[](
	
		vec2(-offset, offset), //top-left
		vec2( 0.0f,   offset), //top-center
		vec2( offset, offset), //top-right
		
		vec2(-offset, 0.0f),   //center-left
		vec2(0.0f, 	  0.0f),   //center-center
		vec2( offset, 0.0f),   //center-right
		
		vec2(-offset, -offset),//bottom-left
		vec2( 0.0f,   -offset),//bottom-center	
		vec2( offset, -offset) //bottom-right
	);
	
	float kernel[9] = float[](
		-1, -1, -1,
		-1,  9, -1,
		-1, -1, -1
	);



	vec3 sample_Tex[9];
	//for(int i = 0; i < 9; i++)
		//sample_Tex[i] = vec3(texture(uTexImage, vUV.st + offsets[i]));
		
	vec3 colour = vec3(0.0f);
	for(int i = 0; i < 9; i++)
		colour += sample_Tex[i] * kernel[i];
		
	FragColour = vec4(colour, 1.0f);


	//FragColour = vec4(texture(uTexImage, vUV).rgb, 1.0f);
	
	vec3 fragPos = texture(uPosition, vUV).rgb;
	vec3 normal = texture(uNormal, vUV).rgb;
	vec3 random_vec = texture(uNoiseTex, vUV*noiseScale).rgb;
	
	vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	
	
	float occlusion = 0.0f;
	for(int i = 0; i < kernelSize; ++i)
	{
		vec3 sample_pos = TBN * uSamples[i];
		sample_pos = fragPos + sample_pos * radius;
		
		vec4 offset = vec4(sample_pos, 1.0f);
		offset = uProjection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5f + 0.5f;
		
		float sample_depth = texture(uPosition, offset.xy).z;
		float range_check = smoothstep(0.0f, 1.0f, radius/abs(fragPos.z - sample_depth));
		occlusion += (sample_depth >= sample_pos.z + bias ? 1.0f : 0.0f) * range_check;
	}
	occlusion = 1.0f - (occlusion / kernelSize);
	FragColour = vec4(occlusion);
}