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
	vec2 texCoord;
}fs_in;


uniform sampler2D uDiffuseMap;

uniform Material uMaterial;
uniform DirectionalLight uDirectionalLight;
uniform bool uPhongRendering;

uniform float uAmbientRatio = 1.0f;

//Functions
vec3 ComputeDirectionalLight(DirectionalLight light, Material mat, vec3 N, vec3 V);

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
	vec3 ambient = uAmbientRatio * light.ambient * mat.ambient;

	//diffuse component
	vec3 Ld = normalize(light.direction);//light direction
	//Lambert cosine law
	float factor = max(dot(N, Ld), 0.0f);
	vec3 diffuse = light.diffuse * mat.diffuse * factor;
	diffuse *=  texture(uDiffuseMap, fs_in.texCoord).rgb;
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
	return ambient + diffuse + specular;
}


