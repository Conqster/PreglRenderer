#pragma once

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <GLM/glm/glm.hpp>

class Shader
{
public:
	Shader() = default;
	bool Create(const std::string& name,  const std::string& ver, const std::string& frag, const std::string& geo = "");

	void Bind() const;

	void SetUniformMat4(const char* name, const glm::mat4& matrix);
	void SetUniform1i(const char* name, int value);
	void SetUniform1f(const char* name, float value);
	void SetUniformVec3(const char* name, const glm::vec3& value);
	void SetUniformVec4(const char* name, const glm::vec4& value);

	void Clear();
private:
	unsigned int mID = 0;
	std::string mName = "unk";
	std::unordered_map<std::string, int> cacheUniformLocations = std::unordered_map<std::string, int>();

	std::string ReadFile(const std::string& shader_file);

	unsigned int CompileShader(GLenum shader_type, const std::string& source);
	int GetUniformLocation(const char* name);
};