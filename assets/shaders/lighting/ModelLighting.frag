#version 400

out vec4 FragColour;

//--------------------Structs ------------------------/
struct Material
{
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
	float shinness;
};

struct DirectionalLight
{
	vec3 direction;
	bool enable;
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
};

//------------------------ Parameters ----------------------------/
in VS_OUT
{
	vec3 fragPos;
	vec3 viewPos;
	vec3 normal;
	vec4 fragPosLightSpace;
}fs_in;


uniform sampler2D utextureMap;
uniform sampler2D uShadowMap;

uniform Material uMaterial;
uniform DirectionalLight uDirectionalLight;
uniform bool uPhongRendering;

//Functions
vec3 ComputeDirectionalLight(DirectionalLight light, Material mat, vec3 N, vec3 V);
float DirShadowCalculation(vec4 shadow_coord);

void main()
{
	//cam view direction
	vec3 V = normalize(fs_in.viewPos - fs_in.fragPos);
	vec3 N = normalize(fs_in.normal);
	
	vec3 result_colour = vec3(0.0f);
	if(uDirectionalLight.enable)
		result_colour += ComputeDirectionalLight(uDirectionalLight, uMaterial, N, V);

	
	//dither and gamma correction
	vec2 dither_coord = gl_FragCoord.xy;// / vec2(textureSize(uMaterial.baseMap, 0));
	float dither = fract(sin(dot(dither_coord, vec2(12.9898, 78.233))) * 43758.5453);
	result_colour += dither * 0.003;
	
	vec3 gamma_corrected = pow(result_colour, vec3(1.0f/2.2f));
	
	FragColour = vec4(result_colour, 1.0f);
	//FragColour = vec4(gamma_corrected, 1.0f);
	

	//FragColour = vec4(uMaterial.diffuse, 1.0f);
}


vec3 ComputeDirectionalLight(DirectionalLight light, Material mat, vec3 N, vec3 V)
{
	vec3 ambient = 0.2f * light.ambient * mat.ambient;

	//diffuse component
	vec3 Ld = normalize(light.direction);//light direction
	//Lambert cosine law
	float factor = max(dot(N, Ld), 0.0f);
	vec3 diffuse = light.diffuse * mat.diffuse * factor;
	
	float specularity = 0.0f;
	//specular component (Blinn-Phong)
	if(!uPhongRendering)
	{
		//halfway 
		vec3 H = normalize(Ld + V);
		specularity = pow(max(dot(N, H), 0.0f), mat.shinness);
	}
	else
	{
		specularity = pow(max(dot(N, Ld), 0.0f), mat.shinness);
	}
	vec3 specular = light.specular * mat.specular * specularity;

	float shadow = DirShadowCalculation(fs_in.fragPosLightSpace);
	return ambient + ((1.0f - shadow) * diffuse + specular);
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