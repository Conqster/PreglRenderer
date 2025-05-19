#include "AppWindow.h"

#include <GL/glew.h>
#include <glfw/glfw3.h>

#include "Core/Log.h"

#include <Windows.h>

#include <functional>

unsigned int AppWindow::mWindowWidth;
unsigned int AppWindow::mWindowHeight;
uint8_t AppWindow::mResizeCallbackCount;
std::array<AppWindow::ResizingCallback, gWindowMaxResizeCallbacks> AppWindow::mResizeCallbackFunctions;

bool AppWindow::Init(const char* name, unsigned int width, unsigned int height, bool full_screen)
{
	PGL_ASSERT_CRITICAL(glfwInit(), "Failed to initialise GLFW!!!!!!");

	mName = name;
	mWindowHeight = height;
	mWindowWidth = width;

	if (!CreateDisplayWindow(name, full_screen))
		return false;


	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK)
	{
		DEBUG_LOG("Glew Init failed, ERROR: ", glewGetErrorString(GlewInitResult));
		glfwDestroyWindow(mWindow);
		glfwTerminate();
		return false;
	}

	return true;
}

void AppWindow::ToggleLockCursor()
{
	glfwSetInputMode(mWindow, GLFW_CURSOR, (mLockCursor = !mLockCursor) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void AppWindow::ChangeWindowTitle(const char* title)
{
	glfwSetWindowTitle(mWindow, title);
	//DEBUG_LOG_INFO("Changed Window Name - ", title);
}

void AppWindow::ChanageWindowName(const char* name)
{
	mName = name;
}

void const AppWindow::SetVSync(bool value)
{
	if (mVSync != value)
	{
		mVSync = value;
		glfwSwapInterval(int(mVSync));
	}
}


void AppWindow::RegisterResizeCallback(std::function<void(unsigned int width, unsigned int height)> new_resize_func_callback, float resize_scale)
{
	if (mResizeCallbackCount < gWindowMaxResizeCallbacks)
		mResizeCallbackFunctions[mResizeCallbackCount++] = { new_resize_func_callback, resize_scale };
	else
		DEBUG_LOG_STATUS("Register Window Callback Reached Limit");
}



void AppWindow::FlushAndSwapBuffer()
{
	glfwSwapBuffers(mWindow);
	glfwPollEvents();
}

bool AppWindow::ProgramWindowActive() const
{
	return !glfwWindowShouldClose(mWindow);
}

void AppWindow::Close() const
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

bool AppWindow::CreateDisplayWindow(const char* name, bool full_screen)
{
	if (full_screen)
	{
		mWindowWidth = GetSystemMetrics(SM_CXSCREEN);
		mWindowHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, name, nullptr, nullptr);
	PGL_ASSERT(mWindow, "Failed to create Window!!!!!!!");

	glfwMakeContextCurrent(mWindow);

	glfwSwapInterval(int(mVSync));

	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(mWindow, OnWindowResizeCallback);
	return true;
}

bool AppWindow::InitGraphicsInterface()
{
	//opengl hint for glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	return false;
}

void AppWindow::OnWindowResizeCallback(GLFWwindow* window, int width, int height)
{
	mWindowHeight = height;
	mWindowWidth = width;
	glViewport(0, 0, mWindowWidth, mWindowHeight);
	for (size_t i = 0; i < mResizeCallbackCount; i++)
	{
		float resize_scale = mResizeCallbackFunctions[i].scale;
		mResizeCallbackFunctions[i].func(static_cast<unsigned int>(mWindowWidth * resize_scale), 
										 static_cast<unsigned int>(mWindowHeight * resize_scale));
	}

	DEBUG_LOG("New Window Display Screen width: ", width, ", height: ", height);
}

