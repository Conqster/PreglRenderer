#pragma once
#include <chrono>

#define LOG_TO_CONSOLE 0

#if LOG_TO_CONSOLE
#include <iostream>
#define CONSOLE_LOG(x) std::cout << x << std::endl
#else
#define CONSOLE_LOG(x)
#endif // DEBUG_TO_CONSOLE

	
//argument scope name, 2nd arg memory address (float) to write scope duration
#define SCOPE_TIME(name, ...) Profiler::TimeTaken scope_time(name, __VA_ARGS__)
								


namespace Profiler
{
	class TimeTaken
	{
		std::chrono::time_point<std::chrono::steady_clock> mStart;
		std::chrono::time_point<std::chrono::steady_clock> mEnd;
		const char* mName;
		std::chrono::duration<float> mDuration;
		float* mDurationMS = nullptr;

	public:
		TimeTaken(const char* name, float* on_destruction_time = nullptr) : mName(name), mDuration(0.0f), mDurationMS(on_destruction_time)
		{
			mStart = std::chrono::high_resolution_clock::now();
		}

		~TimeTaken()
		{
			mEnd = std::chrono::high_resolution_clock::now();
			mDuration = mEnd - mStart;

			if(mDurationMS)
				*mDurationMS = mDuration.count() * 1000.0f;
#if LOG_TO_CONSOLE
			DEBUG_LOG(mName, " - time took: ", mDuration.count() * 1000.0f, "ms.");
#endif // LOG_TO_CONSOLE
			mDurationMS = nullptr;
		}
	}; //TimeTaken struct

} //Profiler namespace