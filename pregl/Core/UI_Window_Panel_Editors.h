#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include <functional>

#include "NonCopyable.h"

struct ImVec2;
class BaseMaterial;

namespace GPUResource {
class Texture;
}
namespace UI {

	void Initialise();
	void ShutDown();

	struct UIFlag
	{
		bool* open = nullptr;
		std::string name;
	};
	void RegisterandGetUIFlag(const char* name, bool* p_open);
#define HELPER_REGISTER_UIFLAG(name, bool_exp_name) \
		static bool intialised = false; \
		static bool bool_exp_name = true; \
		if(!intialised) { \
			UI::RegisterandGetUIFlag(name, &bool_exp_name); \
			intialised = true; \
		}


	std::vector<UIFlag>& GetRegisteredUIFlags();

	namespace Windows {

	

		using MaterialList = std::vector<std::weak_ptr<BaseMaterial>>;
		void MaterialsEditor(MaterialList materials);

		void SingleTextureEditor(GPUResource::Texture& texture);
	} // UI::Windows namespace



	namespace Panels {


		bool TransformQuaternion(glm::mat4& transform, ImVec2 top_left, ImVec2 size);
	} // UI::Panels namespace
} // UI namespace