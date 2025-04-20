#include "ShaderHotReloadTracker.h"

#include "Renderer/Shader.h"
#include "Log.h"


#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>


namespace Util {
	ShaderHotReloadTracker::ShaderHotReloadTracker()
	{
		DEBUG_LOG_STATUS("Hot Reload Tracker Availiable!!!!");
	}
	ShaderHotReloadTracker::~ShaderHotReloadTracker()
	{
		DEBUG_LOG_STATUS("Shader Hot Reloader Closing !!!!!!!!!!!!!!!!!!!!!!");
		if (mTrackingShader.shaderProgram)
			mTrackingShader.shaderProgram = nullptr;
	}
	void ShaderHotReloadTracker::AddShader(Shader* shader)
	{
		PGL_ASSERT_WARN(!shader, "shader null / does not exists.");

		if (shader)
		{
			//record vertex file time stamp
			mTrackingShader = { shader };
			ReflectTimestamp(mTrackingShader);
			DEBUG_LOG_STATUS("Added Shader: ", shader->GetName(), ", ID: ", shader->GetID(), " For Hot Reload Tracking.");
		}
	}

	void ShaderHotReloadTracker::Update()
	{

		if (mTrackingShader.shaderProgram)
		{
			//if one is true update shader, prevent extra check when one has been changed
			//Fragment has more potential to change often
			bool changed = CheckShaderFileChanges(mTrackingShader, ShaderTrackTypeIdx::FRAGMENT);
			
			if(!changed)
				changed = CheckShaderFileChanges(mTrackingShader, ShaderTrackTypeIdx::VERTEX);
			if(!changed)
				changed = CheckShaderFileChanges(mTrackingShader, ShaderTrackTypeIdx::GEOMETRY);

			if (changed)
			{
				auto shader = mTrackingShader.shaderProgram;
				DEBUG_LOG_STATUS("Reload Shader Name: ", shader->GetName(), "ID: ", shader->GetID(), ".");

				auto shader_meta_data = shader->GetMetaData();
				auto path_buffer = shader_meta_data.pathBuffer;
				const char* test = &path_buffer[shader_meta_data.vertexOffset];
				std::string test0 = test;
				bool success = shader->SoftReloadCreate(&path_buffer[shader_meta_data.vertexOffset],
									&path_buffer[shader_meta_data.fragmentOffset],
									&path_buffer[shader_meta_data.geometryOffset]);


				//update timestamps as shader has attempted to reload
				ReflectTimestamp(mTrackingShader);
				DEBUG_LOG_STATUS("Attemped to Reload Shader Name: ", shader->GetName(), "ID: ", shader->GetID(), ".");
			}
		}
		else
		{
			DEBUG_LOG("No Shader to Hot Reload!!!!!!!!\n");
		}
	}

	bool ShaderHotReloadTracker::CheckShaderFileChanges(ShaderFilestamp shader_fs, ShaderTrackTypeIdx shader_idx)
	{
		//definatly need to fix this
		auto& meta_data = mTrackingShader.shaderProgram->GetMetaData();
		auto shader_path_offset = meta_data.fragmentOffset;
		
		switch (shader_idx)
		{
		case Util::ShaderTrackTypeIdx::VERTEX:
			shader_path_offset = meta_data.vertexOffset;
			break;
		case Util::ShaderTrackTypeIdx::GEOMETRY:
			shader_path_offset = meta_data.geometryOffset;
			break;
		default:
			shader_path_offset = meta_data.fragmentOffset;
			break;
		}

		//std::string file_path = &meta_data.buffer[shader_path_offset];
		const char* file_path = &meta_data.pathBuffer[shader_path_offset];

		//"" is use for empty shader, 
		//for instead a shader might have both vert & frag but no geometry shader 
		if (file_path == '\0')
			return false;

		if (!std::filesystem::exists(file_path))
		{
			DEBUG_LOG_WARNING("[CheckShaderFileChanges]: File does not exist !!!, path:", file_path);
			return false;
		}
		auto ftime = std::filesystem::last_write_time(file_path);

		return (ftime != shader_fs.timeStamp[size_t(shader_idx)]);
	}

	time_t ShaderHotReloadTracker::RetrieveTimestamp(std::filesystem::file_time_type time_stamp)
	{
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>
			(time_stamp - std::filesystem::file_time_type::clock::now() +
				std::chrono::system_clock::now());

		return std::chrono::system_clock::to_time_t(sctp);
	}

	void ShaderHotReloadTracker::ReflectTimestamp(ShaderFilestamp& shader_ts)
	{
		auto shader = mTrackingShader.shaderProgram;

		auto shader_meta_data = mTrackingShader.shaderProgram->GetMetaData();
		auto path_buffer = shader_meta_data.pathBuffer;

		bool failed = true;
		auto curr_path = &path_buffer[shader_meta_data.vertexOffset];
		if (std::filesystem::exists(curr_path))
			shader_ts.timeStamp[0] = std::filesystem::last_write_time(curr_path);
		curr_path = &path_buffer[shader_meta_data.fragmentOffset];
		if (std::filesystem::exists(curr_path))
			shader_ts.timeStamp[1] = std::filesystem::last_write_time(curr_path);
		curr_path = &path_buffer[shader_meta_data.geometryOffset];
		if (std::filesystem::exists(curr_path))
			shader_ts.timeStamp[2] = std::filesystem::last_write_time(curr_path);
	}



} //Util namespace