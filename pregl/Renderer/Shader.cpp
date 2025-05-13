#include "Shader.h"
#include "Core/Log.h"
#include <fstream>
#include <GLM/glm/glm.hpp>


bool Shader::Create(const std::string& name, const std::string& ver, const std::string& frag, const std::string& geo)
{
	mName = name;
	mMetaData = CacheMetaData(name, ver, frag, geo);



	std::string vertex_code = ReadFile(ver);
	std::string fragment_code = ReadFile(frag);
	bool has_geometry_shader = !geo.empty();
	std::string geometry_code = (has_geometry_shader) ? ReadFile(geo) : "";

	if (vertex_code.empty() || fragment_code.empty())
	{
		DEBUG_LOG_STATUS("Failed to Create Shader (", mName, "); due empty file.");
		exit(-1);
	}
	
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

	//caches 
	mCacheUniformLocations.reserve(10);

	DEBUG_LOG_STATUS("Complete Shader ", mName, ", GPU ID : ", mID, " Creation");
	return true;
}

bool Shader::SoftReloadCreate(const std::string& ver, const std::string& frag, const std::string& geo)
{
	mMetaData = CacheMetaData(mName, ver, frag, geo);

	std::string vertex_code = ReadFile(ver);
	std::string fragment_code = ReadFile(frag);
	bool has_geometry_shader = !geo.empty();
	std::string geometry_code = (has_geometry_shader) ? ReadFile(geo) : "";

	//check if already loaded
	unsigned new_id = glCreateProgram();
	DEBUG_LOG_STATUS("Try Reloading Shader program, name: ", mName, ", GPU ID: ", mID, ", To --> ID: ", new_id);

	//Compilation process
	GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_code, true);
	GLuint frag_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_code, true);
	GLuint geo_shader = (has_geometry_shader) ? CompileShader(GL_GEOMETRY_SHADER, geometry_code, true) : 0;
	DEBUG_LOG_STATUS("Successfully Recompile Shader Code.");

	// attach complied shader code
	glAttachShader(new_id, vertex_shader);
	glAttachShader(new_id, frag_shader);
	if (has_geometry_shader)
		glAttachShader(new_id, geo_shader);

	
	glLinkProgram(new_id);
	glValidateProgram(new_id);

	GLint valid_shader_progarm = 0;
	glGetProgramiv(new_id, GL_VALIDATE_STATUS, &valid_shader_progarm);

	if (!valid_shader_progarm)
	{
		GLchar log_message[1024];
		glGetShaderInfoLog(new_id, 1024, NULL, log_message);
		DEBUG_LOG_WARNING("[ERROR VALIDATING SHADER PROGRAM (for", mName, "]: ", log_message);


		//need to destroy shader if compiled 
		glDeleteShader(vertex_shader);
		glDeleteShader(frag_shader);
		if (has_geometry_shader)
			glDeleteShader(geo_shader);

		glDeleteProgram(new_id);

		return false;
	}

	//swap id & delete previous
	glDeleteProgram(mID);
	mID = new_id;
	DEBUG_LOG_STATUS("Complete Shader ", mName, ", New GPU ID : ", mID, " Creation");

	glDeleteShader(vertex_shader);
	glDeleteShader(frag_shader);
	if (has_geometry_shader)
		glDeleteShader(geo_shader);

	//caches 
	mCacheUniformLocations.reserve(10);

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

void Shader::SetUniformVec2(const char* name, const glm::vec2& value)
{
	glUniform2f(GetUniformLocation(name), value.x, value.y);
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
	mCacheBindingBlocks.push_back(shader_block);
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

	mCacheUniformLocations.clear();
}

std::string Shader::ReadFile(const std::string& shader_file)
{
	std::string content;
	std::ifstream fileStream(shader_file, std::ios::in);
	if (!fileStream.is_open())
	{
		DEBUG_LOG_WARNING("[Reading Shader File (for ", mName, "]: Failed to read", shader_file, ", file doesn't exist.");
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

unsigned int Shader::CompileShader(GLenum shader_type, const std::string& source, bool soft)
{
	GLuint shaderid = glCreateShader(shader_type);

	const char* src = source.c_str();
	glShaderSource(shaderid, 1, &src, nullptr);
	glCompileShader(shaderid);


	GLint shader_complied = 0;
	glGetShaderiv(shaderid, GL_COMPILE_STATUS, &shader_complied);

	if (!shader_complied)
	{
		GLchar log_message[1024];
		//TO-DO: Need to fix the debug message (for easy debugging)
		//current issue: misspelt a data type (sampler as sample) wrong error message
		//Error message was generic syntax error, unexpected IDENTIFIER, expecting
		// LEFT_BRACE or COMMA or SEMICOLON.
		glGetShaderInfoLog(shaderid, 1024, NULL, log_message);
		DEBUG_LOG_WARNING("[SHADER]: Couldn't create a shader, \n", log_message, "File: ", source);
		
		if(!soft)
			exit(-1);
	}

	return shaderid;
}

int Shader::GetUniformLocation(const char* name)
{
	//fix for multithreading 
	static std::string str_name;
	str_name.reserve(32);
	str_name.assign(name);

	if (mCacheUniformLocations.find(str_name) != mCacheUniformLocations.end()) 
		return mCacheUniformLocations[str_name];
	int location = glGetUniformLocation(mID, name);
	if (location == -1)
		DEBUG_LOG_WARNING("[SHADER UNIFORM (WARNING) program: ", mName, " GPU ID: ", mID, "]: uniform ", name, " doesn't exist!!!!!");
	else
		mCacheUniformLocations[str_name] = location;
	return location;
}

ShaderMetaData Shader::CacheMetaData(const std::string& name, const std::string& ver, const std::string& frag, const std::string& geo)
{
	ShaderMetaData meta{};
	strncpy(meta.name, name.c_str(), sizeof(meta.name));

	//writer helper 
	int write_cursor = 0;
	auto write_meta_buf = [&](const std::string& s, int16_t& offset)
	{
		offset = static_cast<int16_t>(write_cursor);
		std::memcpy(&meta.pathBuffer[write_cursor], s.c_str(), s.size() + 1);
		write_cursor += s.size() + 1;
	};

	write_meta_buf(ver, meta.vertexOffset);
	write_meta_buf(frag, meta.fragmentOffset);
	if(!geo.empty())
		write_meta_buf(geo, meta.geometryOffset);
	return meta;
}
