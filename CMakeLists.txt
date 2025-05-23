cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME PreglRenderer)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(AS_MODULE "Build pregl as a standalone project with its own main" OFF)

#General Properties (Config Properties > General > this)
#output directory
set(CMAKE_BINARY_DIR "${PROJECT_SOURCE_DIR}/Intermediate/\$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Intermediate/\$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Intermediate/\$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
#set(CMAKE_RUNTIME_INTERMEDIATE_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")

include_directories(${PROJECT_SOURCE_DIR})


#add imgui, if required
option(USE_IMGUI "Use ImGui for UI" OFF)

#find all source & library files
file(GLOB_RECURSE source_files "pregl/*.h" "pregl/*.cpp" "samples/*.h" "samples/*.cpp")
file(GLOB_RECURSE library_files "libs/*.h" "libs/*.c" "libs/*.cpp")

set(ALL_SRC_FILES ${source_files} ${library_files})

if(AS_MODULE)
	add_library(${PROJECT_NAME} STATIC ${ALL_SRC_FILES})
else()
	add_executable(${PROJECT_NAME} ${ALL_SRC_FILES})
endif()


#Additional Include Directories (Properties > C/C++ > General > this)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
#external libraries (dependencies)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs/GLM)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs/assimp/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs/glew/include)
#source
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/pregl)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})


#Additional Library Directories (Properties > Linker > General > this)
#Later use command to find cmake for glew add build if x86 or x64 
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs/glfw/lib-vc2022)
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs/glew/lib/Release/x64)
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs/assimp/lib)


if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})										
	target_compile_definitions(${PROJECT_NAME} PUBLIC PGL_ASSETS_PATH="assets")
else()									
	get_filename_component(PGL_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)	
	target_compile_definitions(${PROJECT_NAME} PUBLIC PGL_ASSETS_PATH="${PGL_FOLDER_NAME}/assets")
endif()


#Link Dependencies
target_link_libraries(${PROJECT_NAME} opengl32.lib glew32.lib glfw3.lib assimp-vc143-mtd.lib)

#set project working directory
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(${PROJECT_NAME} PROPERTIES OBJECT_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Intermediate/\$<CONFIG>")

#set CRRT as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})


if(AS_MODULE)
	message(STATUS "pregl as a module")
else()
message(STATUS "pregl as a standalone project")
	#Copy require dll files to output directory (glew32.dll, assimp-vc143-mt.dll)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
					   COMMAND ${CMAKE_COMMAND} -E copy_if_different
						   #copy glew dll file
						   ${PROJECT_SOURCE_DIR}/libs/glew/bin/Release/x64/glew32.dll  
						   #copy assimp dll file
						   ${PROJECT_SOURCE_DIR}/libs/assimp/bin/assimp-vc143-mtd.dll
						   $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

 
