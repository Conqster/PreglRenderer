#version 400

struct Light
{
	vec3 direction;
	vec3 colour;
};

out vec4 FragColour;
in vec3 v_Colour;
in vec2 v_UV;
in vec3 vNormal;
in vec4 vFragPosLightSpace;


uniform sampler2D utextureMap;
uniform sampler2D uShadowMap;
uniform vec4 uColour = vec4(1.0f);

uniform vec3 uviewPos;
uniform Light ulight;

//Functions
float DirShadowCalculation(vec4 shadow_coord);

void main()
{
	vec3 base_colour = texture(utextureMap, v_UV).rgb * uColour.rgb;
	
	vec3 ambient = 0.2f * ulight.colour;
	
	vec3 Ld = normalize(ulight.direction);
	vec3 normal = normalize(vNormal);
	
	float factor = max(dot(normal, Ld), 0.0f);
	vec3 diffuse = ulight.colour * factor * 0.7f;
	vec3 halfway = normalize(Ld + uviewPos);
	vec3 specular = ulight.colour * pow(max(dot(normal, halfway), 0.0f), 64.0f) * 0.5f;
	
	float shadow = DirShadowCalculation(vFragPosLightSpace);
	
	base_colour *= ambient + ((1.0f - shadow) * diffuse + specular);
	//base_colour = vec3(specular * 10.0f);
	FragColour = vec4(abs(v_Colour), 1.0f);
	
	//FragColour = 
	
	FragColour = vec4(abs(normal), 1.0f);
	FragColour = vec4(base_colour, uColour.a);
}




float DirShadowCalculation(vec4 shadow_coord)
{
	//project texture coordinate & fecth the center sample
	vec3 p = shadow_coord.xyz / shadow_coord.w;
	
	p = p * 0.5f + 0.5f;
	
	//Using PCF
    float shadow = 0.0f;
	float bias = 0.00f;
    vec2 texelSize = 1.0f / textureSize(uShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(uShadowMap, p.xy + vec2(x, y) * texelSize).r;
            shadow += p.z - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 10.0f;
    
    return shadow;
}