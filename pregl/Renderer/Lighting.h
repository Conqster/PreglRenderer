#pragma once

#include <glm/glm.hpp>




namespace GPUResource {
	class UniformBuffer;
}

namespace Lighting {

	struct Base
	{
		glm::vec3 ambient{ 0.2f, 0.2f, 0.2f };
		glm::vec3 diffuse{ 0.35f, 0.35f, 0.35f };
		glm::vec3 specular{ 0.75f, 0.75f, .75f };
		bool enable = true; //bool for user friendly & imgui but int for GPU
	};

	struct Directional
	{
		Directional(glm::vec3 dir = glm::vec3(0.0f, -1.0f, 0.0f)) : direction(dir){}
		Directional(glm::vec3 dir, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec) : direction(dir)
		{
			base = Base{ amb, diff, spec };
		}
		Base base;
		glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
		void UpdateUniformBufferData(class GPUResource::UniformBuffer& ubo, unsigned int& offset_pointer);

		static long long int GetGPUShaderSize()
		{
			//shader buffer alignment 
			//ambient colour vec3  // 12 bytes (use 16 bytes)
			//diffuse colour vec3  // 12 bytes (use 16 bytes)
			//specular colour vec3  // 12 bytes (use 16 bytes)
			// 
			//direction vec3	 // 12 bytes r 4 
			//enable flag (int)  // << 4 (complete 16 bytes)

			const unsigned int vec3_size = 3 * sizeof(float); //12 bytes
			const unsigned int int_size = sizeof(int); //4 bytes

			return ((3 * (vec3_size + int_size)) +  // 3 x 16 bytes
					vec3_size +						// 12 bytes dir
					int_size);						// 4 bytes enable flag 
		}
	};

	struct Point
	{
		Point(glm::vec3 point = glm::vec3(0.0f)) : position(point){}
		Base base;
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		void UpdateUniformBufferData(class GPUResource::UniformBuffer& ubo, unsigned int& offset_pointer);
	};

	struct Spot 
	{
		Spot(glm::vec3 point = glm::vec3(0.0f), glm::vec3 dir = glm::vec3(0.0f, -1.0f, 0.0f)) : position(point), direction(dir) {}
		Base base;
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
		float innerCutoffAngle = 25.0f;
		float outerCutoffAngle = 33.0f;
		void UpdateUniformBufferData(class GPUResource::UniformBuffer& ubo, unsigned int& offset_pointer);
	};
} // Light

