#pragma once
#include <memory>
#include <filesystem>

#include "NonCopyable.h"

#include <array>


class Shader;
namespace Util{


	enum class ShaderTrackTypeIdx : uint8_t
	{
		VERTEX = 0U,
		FRAGMENT,
		GEOMETRY,

		COUNT,
	};
	struct ShaderFilestamp
	{
		Shader* shaderProgram = nullptr;
		//vertex, frag, geo
		std::filesystem::file_time_type timeStamp[size_t(ShaderTrackTypeIdx::COUNT)];
	};


	constexpr uint8_t gMaxTrackingShader = 10;
	class ShaderHotReloadTracker : public NonCopyable
	{
	public:
		ShaderHotReloadTracker();
		~ShaderHotReloadTracker();

		void AddShader(Shader* shader);
		void Update();
	private:
		//ShaderFilestamp mTrackingShader;

		uint8_t mShaderCount = 0;
		std::array<ShaderFilestamp, gMaxTrackingShader> mTrackingShaders;

		//utilises
		bool CheckShaderFileChanges(ShaderFilestamp shader_fs, ShaderTrackTypeIdx shader_idx);
		time_t RetrieveTimestamp(std::filesystem::file_time_type time_stamp);
		void ReflectTimestamp(ShaderFilestamp& shader_fs);
	};
} // Util namespace
