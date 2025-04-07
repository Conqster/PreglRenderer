#pragma once

#include "Renderer/GPUVertexData.h"
#include "Core/Log.h"

#include <string>
#include <fstream>
#include <sstream>

#define DEBUG_TO_CONSOLE 0


#if DEBUG_TO_CONSOLE
	#define LOG_STEP_INFO(...) DEBUG_LOG_INFO(...)
#else
	#define LOG_STEP_INFO(...)
#endif // DEBUG_TO_CONSOLE


namespace Loader
{
	inline RenderableMesh LoadMesh(const std::string& path)
	{
		std::ifstream file_stream(path, std::ios::in);
		if (!file_stream.is_open())
		{
			DEBUG_LOG_WARNING("[Loader Mesh]: Failed to open file: ", path);
			return {};
		}

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		int has_colour = 0;
		std::string line;
		while (std::getline(file_stream, line))
		{
			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;
			LOG_STEP_INFO(prefix);
			if (prefix == "vcount")
			{
				int c;
				iss >> c;
				vertices.reserve(c);
				LOG_STEP_INFO("vertex count: ", c);
			}
			else if (prefix == "hascolour")
			{
				iss >> has_colour;
				LOG_STEP_INFO("vertex has colour: ", ((has_colour)? "true":"false"));
			}
			else if (prefix == "icount")
			{
				int c;
				iss >> c;
				indices.reserve(c);
				LOG_STEP_INFO("indices count: ", c);
			}
			else if (prefix == "v")
			{
				Vertex v;
				iss >> v.position[0] >> v.position[1] >> v.position[2] >> 
					   v.normal[0] >> v.normal[1] >> v.normal[2] >> 
					   v.uv[0] >> v.uv[1];
				if(has_colour)
					iss >> v.colour[0] >> v.colour[1] >> v.colour[2];
				else
				{
					v.colour[0] = v.position[0];
					v.colour[1] = v.position[1];
					v.colour[2] = v.position[2];
				}
					
				vertices.push_back(v);
				LOG_STEP_INFO("vertex x: ", v.position[0], "f, y: ", v.position[1], "f, z : ", v.colour[0], "f, r : ", v.colour[1], "f, g : ", v.colour[1], "f, b : ", v.colour[2], "f");
			}
			else if (prefix == "i")
			{
				unsigned int i[3];
				iss >> i[0] >> i[1] >> i[2];
				indices.push_back(i[0]);
				indices.push_back(i[1]);
				indices.push_back(i[2]);
				LOG_STEP_INFO("triange indices: ", i[0], ", ", i[1], ", ", i[2]);
			}
		}

		DEBUG_LOG_STATUS("Complete mesh loading from file. path: ", path);
		RenderableMesh renderable_mesh(vertices, indices);
		return renderable_mesh;
	}


}//Loader namespace