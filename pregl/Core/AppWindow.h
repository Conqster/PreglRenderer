#pragma once
#include <string>

#include <functional>
#include <array>

///Summary 
// app_window --> target window
// class_fun_signature --> callback function symbol  &GPUResource::MultiRenderTarget::ResizeBuffer (ResizeBuffer(unsigned int width, unsigned int height))
// class_inst --> class variable instance 
// resize_scale --> usually (1.0f) using normal window resize but useful for downsampling or upsampling buffers eg 
// eg 0.5f halves the resolution when Resizing (use case SSAO frame buffer) etc
#define REGISTER_RESIZE_CALLBACK_HELPER(app_window, class_func_signature, class_inst, resize_scale) \
		app_window.RegisterResizeCallback(std::bind(class_func_signature, class_inst, std::placeholders::_1, std::placeholders::_2), resize_scale);

//change name later
struct GLFWwindow;
constexpr int gWindowMaxResizeCallbacks = 5;
class AppWindow
{
public:
	AppWindow() = default;
	bool Init(const char* name, unsigned int width, unsigned int height, bool full_screen = false);

	GLFWwindow* GetWindow() const { return mWindow; }
	std::string& GetName() { return mName; }
	unsigned int GetWidth() { return  mWindowWidth; }
	unsigned int GetHeight() { return mWindowHeight; }
	float GetAspectRatio() {return static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight);}

	void ToggleLockCursor();
	inline bool const GetLockCursor() const { return mLockCursor; }

	void ChangeWindowTitle(const char* name);
	void ChanageWindowName(const char* name);
	inline bool GetVSync() const { return mVSync; }
	void const SetVSync(bool value);

	void RegisterResizeCallback(std::function<void(unsigned int width, unsigned int height)> new_resize_func_callback, float resize_scale);

private:
	friend class Application;
	//only friend i.e special programs can flush and swap buffer
	void FlushAndSwapBuffer();
	bool ProgramWindowActive() const;
	void Close() const;

	GLFWwindow* mWindow = nullptr;
	std::string mName = "unk";

	static unsigned int mWindowWidth;
	static unsigned int mWindowHeight;

	bool mLockCursor = true;
	bool mVSync = true;

	bool CreateDisplayWindow(const char* name, bool full_screen = false);
	bool InitGraphicsInterface();

	static void OnWindowResizeCallback(GLFWwindow* window, int width, int height);

	static uint8_t mResizeCallbackCount;
	struct ResizingCallback
	{
		std::function<void(unsigned int width, unsigned int height)> func;
		float scale;
	};
	static std::array<ResizingCallback, gWindowMaxResizeCallbacks> mResizeCallbackFunctions;
};
