#pragma once
#include <string>

//change name later
struct GLFWwindow;
class DisplayManager
{
public:
	DisplayManager() = default;
	bool Init(const char* name, unsigned int width, unsigned int height, bool full_screen = false);

	GLFWwindow* GetWindow() const { return mWindow; }
	std::string GetName() const { return mName; }
	unsigned int GetWidth() { return  mWindowWidth; }
	unsigned int GetHeight() { return mWindowHeight; }
	float GetAspectRatio() {return (float)mWindowWidth / (float)mWindowHeight;}

	void ToggleLockCursor();
	inline bool const GetLockCursor() const { return mLockCursor; }

	void ChangeWindowTitle(const char* name);
	void ChanageWindowName(const char* name);
	inline bool GetVSync() const { return mVSync; }
	void const SetVSync(bool value);

	void FlushAndSwapBuffer();
	/*inline */bool ProgramWindowActive() const;
	void Close() const;
private:
	GLFWwindow* mWindow = nullptr;
	std::string mName = "unk";

	static unsigned int mWindowWidth;
	static unsigned int mWindowHeight;



	bool mLockCursor = true;
	bool mVSync = true;

	bool CreateDisplayWindow(const char* name, bool full_screen = false);
	bool InitGraphicsInterface();

	static void OnWindowResizeCallback(GLFWwindow* window, int width, int height);
};
