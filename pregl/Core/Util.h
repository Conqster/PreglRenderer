#pragma once

#include "Renderer/GPUVertexData.h"
#include <GLM/glm/glm.hpp>



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


} //Util namespace




