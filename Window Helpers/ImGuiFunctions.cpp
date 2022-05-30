#include "ImGuiFunctions.h"

//-----------------------------------------------------------------------------------------------------
// ImGui FUNCTIONS
//-----------------------------------------------------------------------------------------------------

void ImGuide::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
}

void ImGuide::EnableConfigFlags(ImGuiConfigFlags flags)
{
	io->ConfigFlags |= flags;
}

bool ImGuide::InitBackends(GLFWwindow* window)
{
	MainWindow = window;

	return MainWindow == NULL ? false : ( !ImGui_ImplGlfw_InitForOpenGL(MainWindow, true) ? false : ImGui_ImplOpenGL3_Init("#version 330") );
}

void ImGuide::ImGuiNewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuide::SetTheme(ImGuiThemes theme)
{
	switch (theme)
	{
	case     ImGuiThemes_Dark:  ImGui::StyleColorsDark();      break;
	case     ImGuiThemes_Light: ImGui::StyleColorsClassic();   break;
	default: break;
	}
}

void ImGuide::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



//-----------------------------------------------------------------------------------------------------
// OpenGL and GLFW FUNCTONS
//-----------------------------------------------------------------------------------------------------

bool OGLSet::InitGLFW(const char* window_title, int _h, int _w)
{
	if (!glfwInit()) {
		return false;
	}

	// Tell GLFW what version of OpenGL we are using
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window_height = _h;
	window_width  = _w;
	MainWindow = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
	if (MainWindow == NULL) {
		glfwTerminate();
		return false;
	}

	MainMonitor = glfwGetPrimaryMonitor();
	
	// Introduce the window into the current context
	glfwMakeContextCurrent(MainWindow);

	//Load GLAD so it configures OpenGL
	gladLoadGL();

	glViewport(0, 0, _w, _h);

	return true;
}

void OGLSet::SetWindowPos(int _x, int _y)
{
	glfwSetWindowPos(MainWindow, _x, _y);
}

void OGLSet::SetWindowSize(int _w, int _h)
{
	glfwSetWindowSize(MainWindow, _w, _h);
}

void OGLSet::SetWindowCentered()
{
	int win_x, win_y;
	glfwGetMonitorWorkarea(MainMonitor, NULL, NULL, &win_x, &win_y);
	glfwSetWindowPos(MainWindow, (win_x - window_width) / 2, (win_y + 20 - window_height) / 2);	
}

bool OGLSet::IsWindowClosed()
{
	return glfwWindowShouldClose(MainWindow);
}

GLFWwindow* OGLSet::GetWindow()
{
	return MainWindow;
}

void OGLSet::Shutdown()
{
	glfwDestroyWindow(MainWindow);
	glfwTerminate();
}

//-----------------------------------------------------------------------------------------------------
// JUST FUNCTIONS
//-----------------------------------------------------------------------------------------------------

// Rendering Function
void ImGuiOGL_Render(GLFWwindow* window, const ImVec4& BG_Color)
{
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(BG_Color.x * BG_Color.w, BG_Color.y * BG_Color.w, BG_Color.z * BG_Color.w, BG_Color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
}
