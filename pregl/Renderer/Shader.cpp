#include "Shader.h"
#include "Core/Log.h"
#include <fstream>
#include <GLM/glm/glm.hpp>


bool Shader::Create(const std::string& name, const std::string& ver, const std::string& frag, const std::string& geo)
{
	mName = name;
	std::string vertex_code = ReadFile(ver);
	std::string fragment_code = ReadFile(frag);
	bool has_geometry_shader = !geo.empty();
	std::string geometry_code = (has_geometry_shader) ? ReadFile(geo) : "";


	
	mID = glCreateProgram();
	DEBUG_LOG_STATUS("Created Shader program, name: ", mName, ", GPU ID: ", mID);

	//Compilation process
	DEBUG_LOG_STATUS("Compiling Shader ", mName, ", GPU ID : ", mID);
	GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_code);
	GLuint frag_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_code);
	GLuint geo_shader = (has_geometry_shader) ? CompileShader(GL_GEOMETRY_SHADER, geometry_code) : 0;

	glAttachShader(mID, vertex_shader);
	glAttachShader(mID, frag_shader);
	if (has_geometry_shader)
		glAttachShader(mID, geo_shader);

	glLinkProgram(mID);
	glValidateProgram(mID);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	glGetProgramiv(mID, GL_VALIDATE_STATUS, &result);

	if (!result)
	{
		glGetProgramInfoLog(mID, sizeof(eLog), NULL, eLog);
		DEBUG_LOG_WARNING("[ERROR VALIDATING SHADER PROGRAM (for", mName, "]: ", eLog);


		//need to destroy shader if compiled 
		glDeleteShader(vertex_shader);
		glDeleteShader(frag_shader);
		if (has_geometry_shader)
			glDeleteShader(geo_shader);

		return false;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(frag_shader);
	if (has_geometry_shader)
		glDeleteShader(geo_shader);

	DEBUG_LOG_STATUS("Complete Shader ", mName, ", GPU ID : ", mID, " Creation");
	return true;
}

void Shader::Bind() const
{
	glUseProgram(mID);
}

void Shader::SetUniformMat4(const char* name, const glm::mat4& matrix)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniform1i(const char* name, int value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const char* name, float value)
{
	glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniformVec3(const char* name, const glm::vec3& value)
{
	glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void Shader::SetUniformVec4(const char* name, const glm::vec4& value)
{
	glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void Shader::SetUniformBlockIdx(const char* name, int blockBindingIdx)
{
	const int idx = glGetUniformBlockIndex(mID, name);
	glUniformBlockBinding(mID, idx, blockBindingIdx);

	ShaderBlockingIdx shader_block
	{
		name,
		blockBindingIdx
	};
	cacheBindingBlocks.push_back(shader_block);
	DEBUG_LOG_STATUS("[SHADER - program '", mName, "' - ID ", mID, "]: Set new uniform block block name: ", shader_block.name, ", idx: ", shader_block.idx, ".");
}

void Shader::Clear()
{
	DEBUG_LOG_STATUS("Destroying Shader ", mName, "GPU ID: ", mID, ".");
	//TO-DO: not too sure but a program could accually been 0
	if (mID != 0)
	{
		glDeleteProgram(mID);
		mID = 0;
	}
	cacheUniformLocations.clear();
}

std::string Shader::ReadFile(const std::string& shader_file)
{
	std::string content;
	std::ifstream fileStream(shader_file, std::ios::in);
	if (!fileStream.is_open())
	{
		DEBUG_LOG_WARNING("[Reading Shader File (for", mName, "]: Failed to read", shader_file, ", file doesn't exist.");
		return "";
	}
	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

unsigned int Shader::CompileShader(GLenum shader_type, const std::string& source)
{
	GLuint shaderid = glCreateShader(shader_type);

	const char* src = source.c_str();
	glShaderSource(shaderid, 1, &src, nullptr);
	glCompileShader(shaderid);


	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	glGetShaderiv(shaderid, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		//TO-DO: Need to fix the debug message (for easy debugging)
		//current issue: misspelt a data type (sampler as sample) wrong error message
		//Error message was generic syntax error, unexpected IDENTIFIER, expecting
		// LEFT_BRACE or COMMA or SEMICOLON.
		glGetShaderInfoLog(shaderid, sizeof(eLog), NULL, eLog);
		DEBUG_LOG_WARNING("[SHADER]: Couldn't create a shader, \n", eLog, "File: ", source);
		
		exit(-1);
	}

	return shaderid;
}

int Shader::GetUniformLocation(const char* name)
{
	if (cacheUniformLocations.find(name) != cacheUniformLocations.end())
		return cacheUniformLocations[name];


	int location = glGetUniformLocation(mID, name);

	if (location == -1)
		DEBUG_LOG_WARNING("[SHADER UNIFORM (WARNING) program: ", mName, " GPU ID: ", mID, "]: uniform ", name, " doesn't exist!!!!!");
	else
		cacheUniformLocations[name] = location;


	return location;
}
