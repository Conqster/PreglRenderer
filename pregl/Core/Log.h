#pragma once


#define ENABLE_CONSOLE_LOGGING 1
#define ONLY_HIGH_LOG 0
#define DEBUG_SEVERITY_LVL 1 //shows the severity level

#include <stdint.h>

#if ENABLE_CONSOLE_LOGGING
#include <iostream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#define ANSI_RED_COLOUR "\033[1;31m"	   //4
#define ANSI_GREEN_COLOUR "\033[1;32m"	   //2
#define ANSI_YELLOW_COLOUR "\033[1;33m"	   //3
#define ANSI_BLUE_COLOUR "\033[1;34m"      //1
#define ANSI_RESET_COLOUR "\033[0m"
#endif // ENABLE_CONSOLE_LOGGING



namespace Debug {

	enum class SEVERITY : uint8_t
	{
		UNKNOWN = 0,
		INFO,        //<----- Low as for basic logging infomation like "Player pos: {0.0, 0.0, 0.0}, Debug::Log("Position: ", pos);
		STATUS,		 //<----- Low/Medium for logging program status like "Loading Texture ID: 0", Debug::LogStatus("Loading Texture ID: ", 42);
		WARNING,	 //<----- Medium/High (experince performance) for logging infomation "Player health: -100, Debug::LogWarning("player health: ", -100), Debug::LogWarning("High memory usage: ", 96.7f, "%"); 
		ERROR		 //<----- High (system performance) for logging error that affects sys, memeory, format, out of range, Debug::LogError("Shader failed to compile").
	};


	//static std::string SeverityToString(const SEVERITY& value)
	constexpr const char* SeverityToString(const SEVERITY& value)
	{
		switch (value)
		{
		case SEVERITY::UNKNOWN:
			return "Unknown";
		case SEVERITY::INFO:
			return "INFO";
		case SEVERITY::STATUS:
			return "STATUS";
		case SEVERITY::WARNING:
			return "WARNING";
		case SEVERITY::ERROR:
			return "ERROR";
		}

		return "[INVALID SEVERITY LOG]";
	}

	constexpr const char* GetSeverityLvlANSIColour(int lvl = 0)
	{
		switch (lvl)
		{
		case 0U:
			return ANSI_RESET_COLOUR;
		case 1U:
			return ANSI_BLUE_COLOUR;		//1
		case 2U:
			return ANSI_GREEN_COLOUR;	    //2
		case 3U:
			return ANSI_YELLOW_COLOUR;	    //3
		case 4U:
			return ANSI_RED_COLOUR; 	    //4
		}

		return ANSI_RESET_COLOUR;
	}

	//detail are {Debug} internal used information
	namespace detail {

		template<glm::length_t L, typename T, glm::qualifier Q>
		inline std::ostream& operator<<(std::ostream& os, const glm::vec<L, T, Q>& vector)
		{
			return os << glm::to_string(vector);
		}

		inline void ConsoleLog(std::string& msg, const SEVERITY& severity = SEVERITY::UNKNOWN)
		{
#if DEBUG_SEVERITY_LVL
			std::cout << GetSeverityLvlANSIColour((int)severity) << "[" << SeverityToString(severity) << "]: " << ANSI_RESET_COLOUR << msg << "\n";
#else
			std::cout << msg << "\n";
#endif // DEBUG_SEVERITY_LVL
			printf("[USE OLD CONSOLE LOG SYSTEM] size: %d, & .size(): %d.\n", int(sizeof(msg)), int(msg.size()));
		}


		template <typename A, typename... N>
		inline void ConsoleLog(const SEVERITY& severity, A&& a, N&&... n)
		{
			auto& out = std::cout;
#if DEBUG_SEVERITY_LVL
			out << GetSeverityLvlANSIColour((int)severity)
				<< "[" << SeverityToString(severity) << "]: "
				<< ANSI_RESET_COLOUR;
#endif // DEBUG_SEVERITY_LVL
			out << std::forward<A>(a);
			((out << std::forward<N>(n)), ...);
			out << "\n";
		}
	}


	//compile time verification
#if ENABLE_CONSOLE_LOGGING

	//implementations
//#define DEBUG_LOG_IMPL(type, ...) Debug::detail::ConsoleLog(Debug::detail::ToLogString(__VA_ARGS__), type)
#define DEBUG_LOG_IMPL(type, ...) Debug::detail::ConsoleLog(type, __VA_ARGS__)
	constexpr bool ShouldLogSeverity(const Debug::SEVERITY& s);  //<--- forward function declare, see below for decleration
#define INTERNAL_CONSOLE_DEBUG_LOG_CHECK_IMPL(type, ...) \
		if constexpr (ShouldLogSeverity(type)) \
			Debug::detail::ConsoleLog(type, __VA_ARGS__)
			//Debug::detail::ConsoleLog(Debug::detail::ToLogString(__VA_ARGS__), type)

#if ONLY_HIGH_LOG
#define DEBUG_LOG(...)
#define DEBUG_LOG_INFO(...)
#define DEBUG_LOG_STATUS(...) DEBUG_LOG_IMPL(Debug::SEVERITY::STATUS, __VA_ARGS__)
#define DEBUG_LOG_WARNING(...) DEBUG_LOG_IMPL(Debug::SEVERITY::WARNING, __VA_ARGS__)
#define DEBUG_LOG_ERROR(...) DEBUG_LOG_IMPL(Debug::SEVERITY::ERROR, __VA_ARGS__)

#else
#define DEBUG_LOG(...) DEBUG_LOG_IMPL(Debug::SEVERITY::INFO, __VA_ARGS__)
#define DEBUG_LOG_INFO(...) DEBUG_LOG_IMPL(Debug::SEVERITY::INFO, __VA_ARGS__)
#define DEBUG_LOG_STATUS(...) DEBUG_LOG_IMPL(Debug::SEVERITY::STATUS, __VA_ARGS__)
#define DEBUG_LOG_WARNING(...) DEBUG_LOG_IMPL(Debug::SEVERITY::WARNING, __VA_ARGS__)
#define DEBUG_LOG_ERROR(...) DEBUG_LOG_IMPL(Debug::SEVERITY::ERROR, __VA_ARGS__)
#endif // ONLY_HIGH_LOG

	//defination
	constexpr bool ShouldLogSeverity(const Debug::SEVERITY& s)
	{
#if ONLY_HIGH_LOG
		return s >= Debug::SEVERITY::STATUS;
#else
		return true;
#endif // ONLY_HIGH_LOG
	}
#else
#define INTERNAL_CONSOLE_DEBUG_LOG_CHECK_IMPL(type, msg) (0) 
#define DEBUG_LOG_IMPL(type, ...) (0)
#define DEBUG_LOG(...) (0)
#define DEBUG_LOG_INFO(...) (0) 
#define DEBUG_LOG_STATUS(...) (0)
#define DEBUG_LOG_WARNING(...) (0)
#endif // ENABLE_CONSOLE_LOGGING


	template<SEVERITY severity, typename ...Args>
	inline static void Log( Args && ...args)
	{
		INTERNAL_CONSOLE_DEBUG_LOG_CHECK_IMPL(severity, args...);
	}

	template<typename ...Args>
	inline static void Log(Args && ...args)
	{
		INTERNAL_CONSOLE_DEBUG_LOG_CHECK_IMPL(SEVERITY::INFO, args...);
	}


	template<typename... Args>
	inline static void LogStatus(Args&& ...args)
	{
		INTERNAL_CONSOLE_DEBUG_LOG_CHECK_IMPL(SEVERITY::STATUS, args...);
	}

	template<typename... Args>
	inline static void LogWarning(Args&& ...args)
	{
		INTERNAL_CONSOLE_DEBUG_LOG_CHECK_IMPL(SEVERITY::WARNING, args...);
	}


	template<typename... Args>
	inline static void LogError(Args&& ...args)
	{
		INTERNAL_CONSOLE_DEBUG_LOG_CHECK_IMPL(SEVERITY::ERROR, args...);
	}

}





enum class ASSERT_TYPE : uint8_t
{
	WARNING = 3,
	CRITICAL,
};



#ifdef _DEBUG
constexpr const char* GetAssertName(const ASSERT_TYPE& type)
{
	return (type == ASSERT_TYPE::CRITICAL) ? "CRITICAL" : "WARNING"; 
}

template<typename... Args>
[[maybe_unused]] inline static void AssertOutput(const char* expr, const ASSERT_TYPE& type, const char* file, unsigned line, Args&&... msg_args)
{
	auto& out = std::cout;
	out << Debug::GetSeverityLvlANSIColour((int)type) << "[" << GetAssertName(type) << "]: " << ANSI_RESET_COLOUR;

	out << "Assertion failed (" << expr << ").";
	
	((out << std::forward<Args>(msg_args)), ...);

	out << ". FILE: " << file
		<< ", LINE: " << line << std::endl;

}


#define PGL_INTERNAL_ASSERT_IMPL(expr, type, ...) \
			do { \
				if(!(expr)) { \
					AssertOutput(#expr, type, __FILE__, __LINE__, __VA_ARGS__); \
					if(type == ASSERT_TYPE::CRITICAL) __debugbreak(); \
				} \
			}while(0)

#define PGL_ASSERT(expr, ...) PGL_INTERNAL_ASSERT_IMPL(expr, ASSERT_TYPE::CRITICAL, __VA_ARGS__)
#define PGL_ASSERT_CRITICAL(expr, ...) PGL_INTERNAL_ASSERT_IMPL(expr, ASSERT_TYPE::CRITICAL, __VA_ARGS__)
#define PGL_ASSERT_WARN(expr, ...) PGL_INTERNAL_ASSERT_IMPL(expr, ASSERT_TYPE::WARNING, __VA_ARGS__)

#else
#define PGL_ASSERT(expr, ...) (0)
#define PGL_INTERNAL_ASSERT_IMPL(expr, type, ...) (0) 
#define PGL_ASSERT_CRITICAL(expr, ...) (0)
#define PGL_ASSERT_WARN(expr, ...) (0)
#endif // DEBUG

