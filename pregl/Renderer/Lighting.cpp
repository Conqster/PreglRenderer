#include "Lighting.h"
#include "Renderer/GPUResources.h"

namespace Lighting {

	void Directional::UpdateUniformBufferData(GPUResource::UniformBuffer& ubo, unsigned int& offset_pointer)
	{
		//shader buffer alignment 
		//ambient_light vec3  // 12 bytes (use 16 bytes)
		//diffuse_light vec3  // 12 bytes (use 16 bytes)
		//specular_light vec3  // 12 bytes (use 16 bytes)
		//direction vec3	 // 12 bytes r 4 
		//enable flag (int)  // << 4 (complete 16 bytes)

		unsigned int vec3_size = 3 * sizeof(float); 
		unsigned int int_size = sizeof(int); 

		//ambient colour // 12 bytes (use 16 bytes)
		ubo.SetSubDataByID(&base.ambient[0], vec3_size, offset_pointer);
		offset_pointer += vec3_size + int_size; //compensation for required 16 buffer slot
		//diffuse colour // 12 bytes (use 16 bytes)
		ubo.SetSubDataByID(&base.diffuse[0], vec3_size, offset_pointer);
		offset_pointer += vec3_size + int_size;
		//specular colour // 12 bytes (use 16 bytes)
		ubo.SetSubDataByID(&base.specular[0], vec3_size, offset_pointer);
		offset_pointer += vec3_size + int_size;
		//direction 
		ubo.SetSubDataByID(&direction[0], vec3_size, offset_pointer);
		offset_pointer += vec3_size;
		//enable
		ubo.SetSubDataByID(&base.enable, int_size, offset_pointer);
		offset_pointer += int_size;
	}

	void Point::UpdateUniformBufferData(GPUResource::UniformBuffer& ubo, unsigned int& offset_pointer)
	{
	}

	void Spot::UpdateUniformBufferData(GPUResource::UniformBuffer& ubo, unsigned int& offset_pointer)
	{
	}

} // Light
