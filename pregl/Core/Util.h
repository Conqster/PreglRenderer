#pragma once

#include "Renderer/GPUVertexData.h"
#include <GLM/glm/glm.hpp>

#include "Core/Log.h"

#include <random>

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




	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Range
	///////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	struct Range
	{
		T min;
		T max;
		bool IsValid() const
		{
			return min <= max;
		}
		T Mid() const
		{
			return (min + max)/2;
		}
		T Span() const
		{
			return max - min;
		}

		//extras utilities
		T Clamp(T value) const
		{
			return std::min(std::max(value, min), max);
		}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Random
	///////////////////////////////////////////////////////////////////////////////////////////////////
	namespace Random
	{
#define USE_UNIFORM_DISTRIBUTION 1


		static constexpr uint32_t rnd_default_mt = 5489U; //mersenne_twister_engine default seed
		static constexpr uint32_t rnd_b4mt = 4357U; //https://sourceware.org/legacy-ml/gsl-discuss/2006-q4/msg00014.html
		static constexpr uint32_t rnd_universal_42 = 42;
		static constexpr uint32_t rnd_power_123 = 123; //https://www.stata.com/manuals13/rsetseed.pdf


		/* Base Random Number Generator {RNG} class
		 use case global static / quick static instance Float, Int rnd etc
		 Note: Not thread-safe. create seperate instance per thread (if necesasary)
		 Sample:
			auto randomf = Util::TestRandom<float>(0.0f, 1.0f);
		
			//determininstic control
			randomf.Seed(42);
			randomf.Seed(rnd_universal_42);
			randomf.Seed(rnd_default_mt);
			randomf.Seed(rnd_power_123);
		
			//return random value
			randomf.Value() or randomf()
		
			//return instance range value (one off range)
			randomf.ValueWithRange(min, max) */
		template<typename T>
		class RNG
		{
		private:
			std::mt19937 mEngine;
			uint32_t mSeed = rnd_default_mt;
			//std::default_random_engine mEngine;
			//distributions
			//std::uniform_real_distribution<T> mDistRnd;
			//std::normal_distribution<T> mDistRnd;
			//std::student_t_distribution<T> mDistRnd;
			//std::poisson_distribution<T> mDistRnd;
			//std::extreme_value_distribution<T> mDistRnd;

			//distrubution type for floating-point & intergal data types
			using DistributionType = typename std::conditional<
				std::is_integral<T>::value,
				std::uniform_int_distribution<T>,
				std::uniform_real_distribution<T>>::type;
			DistributionType mDistRnd;
		public:
			explicit RNG(T min = T(0), T max = T(1), bool non_deterministic = true) : mEngine(std::mt19937()), mSeed(rnd_default_mt)
			{
				Init(min, max, non_deterministic);
			}
			RNG(uint32_t seed, T min = T(0), T max = T(1)) : mSeed(seed)
			{
				Init(min, max, false);
			}
			void Init(T min, T max, bool non_deterministic = true)
			{
				mSeed = (non_deterministic) ? std::random_device{}() : mSeed;
				mEngine = std::mt19937(mSeed);
				mDistRnd = DistributionType(min, max);
			}

			void SetDistribution(T min, T max)
			{
				mDistRnd = DistributionType(min, max);
			}

			void Seed(uint32_t seed)
			{
				mSeed = seed;
				mEngine.seed(mSeed);
			}

			void Reset(bool non_deterministic = true, uint32_t seed = rnd_default_mt)
			{
				mSeed = seed;
				Init(mDistRnd.min(), mDistRnd.max(), non_deterministic);
			}

			uint32_t GetSeed() const { return mSeed;}
			Range<T> Range() const { return { mDistRnd.min(), mDistRnd.max() };}
			T Value() { return mDistRnd(mEngine); }
			T operator()() { return mDistRnd(mEngine); }

			/// <summary>
			/// Creates instantanous distribution to rnd range;
			/// return rnd with range 
			/// NB: min/max calls use temprary distribution; 
			/// no need to reset internal one.
			/// </summary>
			/// <param name="min"></param>
			/// <param name="max"></param>
			/// <returns></returns>
			T ValueWithRange(T min, T max)  
			{
				DistributionType temp_dist(min, max);
				return temp_dist(mEngine);
			}

			/// <summary>
			/// Creates instantanous distribution to rnd range;
			/// return rnd with range 
			///	NB: min/max calls use temprary distribution; 
			/// no need to reset internal one.
			/// </summary>
			/// <param name="min"></param>
			/// <param name="max"></param>
			/// <returns></returns>
			T operator()(T min, T max)
			{
				DistributionType temp_dist(min, max);
				return temp_dist(mEngine);
			}
		};


#if USE_UNIFORM_DISTRIBUTION

		/* RNG float global static [0, 1]
		* Global uniform GNG in range [0.0f, 1.0f]
		* not advice to set its distrubution as its global state (set to at for universal use across program);
		* if distribution in between frame; return to previous distrubution to ensure normal(/default) behaviour;
		* can retrive & cache range .Range() returns Range<float> 
		* 
		* //  you can retrive and cache its state: 
		* Range<float> cache_range = gRngFloat01.Range();						@get previous range
		* uint32_t cache_seed = gRngFloat01.GetSeed();						@get previous seed
		* 
		* // Then you can moditfy it:
		* gRngFloat01.SetDistribution(-1.0f, 1.0f);							@new range [-1.0f, 1.0f]
		* gRngFloat01.Seed(custom_seed);									@new range [custom]
		* 
		* //generate values:
		* float random_value = gRngFloat01(); or gRndFloat.Value();			@gets random value
		* 
		* //and restore it:
		* gRngFloat01.SetDistribution(cache_range.min, cache_range.max);		@new range [-1.0f, 1.0f]
		* gRngFloat01.Seed(cache_seed);									@new range [custom] */
		static inline RNG<float> gRngFloat01(rnd_b4mt, 0.0f, 1.0f);
		static inline RNG<int> gRngInt(0, 1);
#endif // USE_UNIFORM_DISTRIBUTION


		//quick random value between two value, min & max
		static inline float Float(float min = 0.0f, float max = 1.0f)
		{
#if USE_UNIFORM_DISTRIBUTION
			if (min == 0.0f && max == 1.0f)
				return gRngFloat01();
			else
				return gRngFloat01(min, max);
#else
			return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
#endif // USE_UNIFORM_DISTRIBUTION
		}

		//quick random value between two value, min & max
		static inline int Int(int min = 0, int max = 1)
		{
#if USE_UNIFORM_DISTRIBUTION
			if (min == 0 && max == 1)
				return gRngInt();
			else
				return gRngInt(min, max);
#else
			return min + rand() % (max - min + 1);
#endif // USE_UNIFORM_DISTRIBUTION
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




