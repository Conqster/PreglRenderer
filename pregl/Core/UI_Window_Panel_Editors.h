#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include <functional>

#include "NonCopyable.h"

//#include <array>

struct ImVec2;
class BaseMaterial;

namespace GPUResource {
class Texture;
}
namespace UI {
	//move into manager
	void Initialise();
	void ShutDown();

	struct UIFlag
	{
		bool* open = nullptr;
		std::string name;
	};

	//struct NewUIFlag
	//{
	//	bool open = true;
	//	std::string name;
	//};

	//constexpr int MAX_UI_FLAG = 10;
	//class Manager
	//{
	//private:
	//	using RegisterUIFlags = std::array<NewUIFlag, MAX_UI_FLAG>;
	//	RegisterUIFlags mRegisterUIFlagPool;
	//	size_t mRegisterUICount = 0;
	//public:
	//	static Manager& ManagerInstance() {
	//		static Manager inst;
	//		return inst;
	//	}

	//	bool& GenerateNewUIFlag(const char* ui_name)
	//	{
	//		if (mRegisterUICount < MAX_UI_FLAG)
	//		{
	//			mRegisterUIFlagPool[mRegisterUICount++] = { true, ui_name };
	//			return mRegisterUIFlagPool[mRegisterUICount - 1].open;
	//		}
	//		printf("Failed to Generate New UI Flag!!!!\n");
	//		//need to find a temp bool that would not go out of scope
	//		//quick temporary fallback 
	//		static bool temp = false;
	//		return temp;
	//	}

	//	inline RegisterUIFlags& GetRegisteredUIFlags() { return mRegisterUIFlagPool; }
	//	inline size_t GetRegisteredUICount() { return mRegisterUICount; }
	//};


	void RegisterandGetUIFlag(const char* name, bool* p_open);
#define HELPER_REGISTER_UIFLAG(name, bool_exp_name) \
		static bool intialised = false; \
		static bool bool_exp_name = true; \
		if(!intialised) { \
			UI::RegisterandGetUIFlag(name, &bool_exp_name); \
			intialised = true; \
		}

#define HELPER_REGISTER_UIFLAG_MULTICALL(name_id, flag_name_id) \
		static bool intialised##_##name_id = false; \
		static bool bool_exp_name##_##name_id = true; \
		if(!intialised##_##name_id) { \
			UI::RegisterandGetUIFlag(name_id, &bool_exp_name##_##name_id); \
			intialised##_##name_id = true; \
		}

	std::vector<UIFlag>& GetRegisteredUIFlags();

	namespace Windows {

		using MaterialList = std::vector<std::weak_ptr<BaseMaterial>>;
		void MaterialsEditor(MaterialList materials);

		void SingleTextureEditor(GPUResource::Texture& texture, const char* name_buf, bool* open_flag = nullptr);
	} // UI::Windows namespace


#define HELPER_SINGLE_TEXTURE_EDITOR_UIFLAG(texture, bool_exp_name) \
		char name_buf[64]; \
		snprintf(name_buf, sizeof(name_buf), "Texture Editor Window GPU: %d {WIP}.", int(texture.GetID())); \
		static bool intialised##_##bool_exp_name = false; \
		static bool bool_exp_name = true; \
		if(!intialised##_##bool_exp_name) { \
			UI::RegisterandGetUIFlag(name_buf, &bool_exp_name); \
			intialised##_##bool_exp_name = true; \
		} \
		if(bool_exp_name) UI::Windows::SingleTextureEditor(texture, name_buf, &bool_exp_name);


	namespace Panels {

		bool TransformQuaternion(glm::mat4& transform, ImVec2 top_left, ImVec2 size);
	} // UI::Panels namespace
} // UI namespace