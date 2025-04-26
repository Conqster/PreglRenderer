#pragma once
#include "Core/Log.h"
#include <glm/glm.hpp>
#include <memory>

namespace GPUResource {
	class Texture;
}
struct BaseMaterial
{
	BaseMaterial() { DEBUG_LOG_STATUS("New Material instantiated."); }
	

	const char* name = "base-mat";
	float shinness = 64.0f;
	bool blinn_phong = true;
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);

	//quick hack 
	float opacity = 1.0f;


	std::shared_ptr<GPUResource::Texture> diffuseMap = nullptr;
	std::shared_ptr<GPUResource::Texture> normalMap = nullptr;
	std::shared_ptr<GPUResource::Texture> specularMap = nullptr;

	void Destroy()
	{
		diffuseMap.reset();
		normalMap.reset();
		specularMap.reset();
		DEBUG_LOG_STATUS("Destroied Material ", name);
	}
};