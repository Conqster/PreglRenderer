#pragma once

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <GLM/glm/glm.hpp>

#include <stdint.h>
struct ShaderBlockingIdx
{
	std::string name;
	unsigned int idx;

	//a compare operator to check two block
};

struct ShaderMetaData
{
	char name[32];
	char pathBuffer[256];
	int16_t vertexOffset = 0;
	int16_t fragmentOffset = 0;
	int16_t geometryOffset = -1;
};

class Shader
{
public:
	Shader() = default;
	bool Create(const std::string& name,  const std::string& ver, const std::string& frag, const std::string& geo = "");
	bool SoftReloadCreate(const std::string& ver, const std::string& frag, const std::string& geo = "");

	void Bind() const;

	void SetUniformMat4(const char* name, const glm::mat4& matrix);
	void SetUniform1i(const char* name, int value);
	void SetUniform1f(const char* name, float value);
	void SetUniformVec2(const char* name, const glm::vec2& value);
	void SetUniformVec3(const char* name, const glm::vec3& value);
	void SetUniformVec4(const char* name, const glm::vec4& value);
	void SetUniformBlockIdx(const char* name, int blockBindingIdx = 0);

	std::vector<ShaderBlockingIdx> GetBindingBlocks() const { return mCacheBindingBlocks; }
	const char* GetName() const { return mName.c_str(); }
	unsigned int GetID() const { return mID; }
	ShaderMetaData& GetMetaData() { return mMetaData; }

	void Clear();
private:
	unsigned int mID = 0;
	std::string mName = "unk";

	ShaderMetaData mMetaData;
	//std::unordered_map<std::string, int> mCacheUniformLocations = std::unordered_map<std::string, int>();


	std::unordered_map<std::string, int> mCacheUniformLocations = std::unordered_map<std::string, int>();
	std::vector<ShaderBlockingIdx> mCacheBindingBlocks = std::vector<ShaderBlockingIdx>();

	std::string ReadFile(const std::string& shader_file);

	unsigned int CompileShader(GLenum shader_type, const std::string& source, bool soft = false);
	int GetUniformLocation(const char* name);

	ShaderMetaData CacheMetaData(const std::string& name, const std::string& ver, const std::string& frag, const std::string& geo = "");
};