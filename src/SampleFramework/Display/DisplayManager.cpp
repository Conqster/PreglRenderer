#include "DisplayManager.h"

#include <GL/glew.h>
#include <glfw/glfw3.h>

#include <iostream>

#include <Windows.h>

unsigned int DisplayManager::mWindowWidth;
unsigned int DisplayManager::mWindowHeight;

bool DisplayManager::Init(const char* name, unsigned int width, unsigned int height, bool full_screen)
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialise GLFW!!!!!!\n";
		return false;
	}

	mName = name;
	mWindowHeight = height;
	mWindowWidth = width;

	if (!CreateDisplayWindow(name, full_screen))
		return false;


	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK)
	{
		std::cout << "Glew Init failed, ERROR: " << glewGetErrorString(GlewInitResult) << "\n";
		glfwDestroyWindow(mWindow);
		glfwTerminate();
		return false;
	}

	return true;
}

void DisplayManager::ToggleLockCursor()
{
	glfwSetInputMode(mWindow, GLFW_CURSOR, (mLockCursor = !mLockCursor) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void DisplayManager::ChangeWindowTitle(const char* title)
{
	glfwSetWindowTitle(mWindow, title);
}

void DisplayManager::ChanageWindowName(const char* name)
{
	mName = name;
}

void const DisplayManager::SetVSync(bool value)
{
	if (mVSync != value)
	{
		mVSync = value;
		glfwSwapInterval(int(mVSync));
	}
}

void DisplayManager::FlushAndSwapBuffer()
{
	glfwSwapBuffers(mWindow);
	glfwPollEvents();
}

bool DisplayManager::ProgramWindowActive() const
{
	return !glfwWindowShouldClose(mWindow);
}

void DisplayManager::Close() const
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

bool DisplayManager::CreateDisplayWindow(const char* name, bool full_screen)
{
	if (full_screen)
	{
		mWindowWidth = GetSystemMetrics(SM_CXSCREEN);
		mWindowHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, name, nullptr, nullptr);

	if (!mWindow)
	{
		std::cout << "Failed to create Window!!!!!!!\n";
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(mWindow);

	glfwSwapInterval(int(mVSync));

	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(mWindow, OnWindowResizeCallback);
	return true;
}

bool DisplayManager::InitGraphicsInterface()
{
	//opengl hint for glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	return false;
}

void DisplayManager::OnWindowResizeCallback(GLFWwindow* window, int width, int height)
{
	mWindowHeight = height;
	mWindowWidth = width;
	glViewport(0, 0, mWindowWidth, mWindowHeight);


	printf("New Window Display Screen width: %d, height: %d\n", width, height);
}

