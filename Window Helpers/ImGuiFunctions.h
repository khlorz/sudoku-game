#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImFunks.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum ImGuiThemes_
{
	ImGuiThemes_Dark  = 0,
	ImGuiThemes_Light = 1
};

using ImGuiThemes = int;

class ImGuide
{
private:
	ImGuiIO*    io;
	ImGuiStyle* style;
	GLFWwindow* MainWindow;

public:
	ImGuide()  = default;
	~ImGuide() = default;

	void InitImGui();
	bool InitBackends(GLFWwindow* window);
	void EnableConfigFlags(ImGuiConfigFlags flags);
	void SetTheme(ImGuiThemes theme);
	bool ImGuiFonts();
	void ImGuiNewFrame();
	void Shutdown();

};

class OGLSet
{
private:
	int          window_height;
	int          window_width;
	GLFWwindow*  MainWindow;   // The main window
	GLFWmonitor* MainMonitor;  // The main monitor context

public:
	OGLSet()  = default;
	~OGLSet() = default;

	bool InitGLFW(const char* window_title, int _h, int _w);
	void SetWindowSize(int _w, int _h);
	void SetWindowPos(int _x, int _y);
	void SetWindowCentered();
	bool IsWindowClosed();
	void Shutdown();
	GLFWwindow* GetWindow();

};

// Rendering Function
void ImGuiOGL_Render(GLFWwindow* window, const ImVec4& BG_Color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

