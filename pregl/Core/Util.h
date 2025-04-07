#pragma once

#include "Renderer/GPUVertexData.h"
#include <GLM/glm/glm.hpp>

#include "Core/Log.h"

namespace Util
{
	constexpr double PI = 3.141592653589793238462643;

	static inline RenderableMesh CreateSphere(unsigned int sector_count = 36, unsigned int span_count = 18)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		//vextex position
		float x, y, z, w;
		//vectex texture coord
		float u, v;

		float sector_angle;
		float span_angle;

		//TO-DO: for now use float, double will be too redundant to use 
		float sector_step = (float)(2 * PI / sector_count);  // 0 - 360(2pi)/count =>  angle btw each steps
		float span_step = (float)(PI / span_count);		  // 0 - 180(pi)/count => angle btw each step

		//float radius = 1.0f;
		float radius = 0.5f;

		//compute & store vertices
		for (unsigned int i = 0; i <= span_count; i++)
		{
			// 180 degree(pi) to 0 degree //0 degree to 180 degree(pi)
			span_angle = (float)PI - i * span_step;

			//parametric equation for sphere
			// x = center_x + r * sin(theta) * cos(phi)    
			// y = center_y + r * sin(thata) * sin(phi)
			// z = center_z + r * cos(theta)
			// where 
			//		theta = span_angle range 0 to pi(180 degrees)
			//		phi = sector_angle range 0 to 2pi(360 degrees)
			//RETERIVED: https://en.m.wikipedia.org/wiki/Sphere
			//			In their case z is up axis


			w = radius * glm::sin(span_angle);
			y = radius * glm::cos(span_angle);

			//add (sector_count + 1) vertices per stack
			//first and last vertices have same position, but different tex coords 
			for (unsigned int j = 0; j <= sector_count; ++j)
			{
				//start from 0 >> current step * step angle >> 360 
				sector_angle = j * sector_step;

				//vectex position (x, y, z)
				x = w * glm::cos(sector_angle);			//r * cos(u) * cos(v)
				z = w * glm::sin(sector_angle);			//r * cos(u) * sin(v)

				//vertex texture coord range between [0, 1]
				u = (float)j / sector_count;
				v = (float)i / span_count;

				Vertex vertex
				{
					{x,y, z},
					{x,y, z},
					{u,v},
					{x,y, z}
				};
				vertices.push_back(vertex);
			}
		}


		//compute & store indices
		unsigned int k1, k2;
		for (unsigned int i = 0; i < span_count; ++i)
		{
			k1 = i * (sector_count + 1);		//beginning of current stack
			k2 = k1 + sector_count + 1;			//beginning of next stack

			for (unsigned int j = 0; j < sector_count; ++j, ++k1, ++k2)
			{
				//2 triangles per sector excluding first and last stacks
				//k1 => k2 => k1+1
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				//k1+1 => k2 => k2+ 1
				if (i != (span_count - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}

			}
		}


		DEBUG_LOG_STATUS("Created new sphere vertex data");
		return RenderableMesh(vertices, indices);
	}



	namespace Random
	{
		//quick random value between two value, min & max
		static inline float Float(float min, float max)
		{
			return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
		}


		//quick random value between two value, min & max
		static inline int Int(int min, int max)
		{
			return min + rand() / (RAND_MAX / (max - min));
		}


		//quick random point but not fully uniform 
		static inline glm::vec3 PointInSphere(float radius)
		{
			if (radius <= 0.0f)
				return glm::vec3(0.0f);

			while (true)
			{
				float x = Float(-radius, radius);
				float y = Float(-radius, radius);
				float z = Float(-radius, radius);

				//printf("Random point in sphere x: %f, y: %f, z: %f\n", x, y, z);

				if (x * x + y * y + z * z <= radius * radius)
					return glm::vec3(x, y, z);
			}
		}
	}//Random namespace



	static inline void DecomposeTransform(const glm::mat4& transform, glm::vec3& translate, glm::vec3& euler, glm::vec3& scale)
	{
		glm::mat4 Mt = transform;
		translate = glm::vec3(Mt[3]);

		if (std::abs(Mt[3][3] - 1.0f) > 1e-6f)
			return;

		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (std::isnan(Mt[i][j]))
					return;

		//Remove translation for scale & rot extraction 
		Mt[3] = glm::vec4(0.0f, 0.0f, 0.0f, Mt[3].w);

		scale.x = glm::length(glm::vec3(Mt[0][0], Mt[1][0], Mt[2][0]));
		scale.y = glm::length(glm::vec3(Mt[0][1], Mt[1][1], Mt[2][1]));
		scale.z = glm::length(glm::vec3(Mt[0][2], Mt[1][2], Mt[2][2]));

		if (glm::determinant(Mt) < 0)
			scale.x = -scale.x;

		glm::mat3 Mr;
		Mr[0] = glm::vec3(Mt[0][0], Mt[1][0], Mt[2][0]) / scale.x;
		Mr[1] = glm::vec3(Mt[0][1], Mt[1][1], Mt[2][1]) / scale.y;
		Mr[2] = glm::vec3(Mt[0][2], Mt[1][2], Mt[2][2]) / scale.z;

		euler.x = atan2(Mr[2][1], Mr[2][2]);
		euler.y = atan2(-Mr[2][0], glm::sqrt(Mr[2][1] * Mr[2][1] + Mr[2][2] * Mr[2][2]));
		euler.z = atan2(Mr[1][0], Mr[0][0]);

		euler = glm::degrees(euler);
	}


} //Util namespace




