#include "GameWindow.h"
#include "font.h"
#include "IconsFontAwesome5.h"
#include <filesystem>
#include "ImGuiFunctions.h"

int main()
{
    OGLSet window_helper;
    if (!window_helper.InitGLFW("Sudoku Game", 507, 785)) {
        return -1;
    }
    window_helper.SetWindowCentered();
    glfwSwapInterval(1); // Enable V-sync

    auto main_window_object = window_helper.GetWindow();

    ImGuide imgui_helper;
    imgui_helper.InitImGui();
    if (!imgui_helper.InitBackends(main_window_object)) {
        imgui_helper.Shutdown();
        window_helper.Shutdown();
        return -1;
    }
    imgui_helper.EnableConfigFlags(ImGuiConfigFlags_NavEnableKeyboard);
    imgui_helper.SetTheme(ImGuiThemes_Dark);

    std::string dir = ((std::filesystem::current_path()).string()) + "\\Fonts\\";
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    ImFontConfig CustomFont;
    CustomFont.FontDataOwnedByAtlas = false;

    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 2;
    icons_config.OversampleV = 2;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 14.0f, &CustomFont);
    io.Fonts->AddFontFromFileTTF((dir + FONT_ICON_FILE_NAME_FAR).c_str(), 14.0f, &icons_config, icons_ranges);
    io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 17.0f, &CustomFont);
    io.Fonts->AddFontFromFileTTF((dir + FONT_ICON_FILE_NAME_FAR).c_str(), 17.0f, &icons_config, icons_ranges);
    io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 20.0f, &CustomFont);
    io.Fonts->AddFontFromFileTTF((dir + FONT_ICON_FILE_NAME_FAR).c_str(), 20.0f, &icons_config, icons_ranges);
    io.Fonts->AddFontDefault();

    bool show_demo = true;

    GameWindow gamewindow_helper;
    if (!gamewindow_helper.OneTimeInitialize()) {
        imgui_helper.Shutdown();
        window_helper.Shutdown();
        return -1;
    }

    while (!window_helper.IsWindowClosed()) {
        glfwPollEvents();
        imgui_helper.ImGuiNewFrame();

        //ImGui::ShowDemoWindow();

        gamewindow_helper.RenderWindow();
        if (gamewindow_helper.IsWindowClosed()) {
            glfwSetWindowShouldClose(main_window_object, GLFW_TRUE);
        }

        ImGuiOGL_Render(main_window_object);
    }

    imgui_helper.Shutdown();
    window_helper.Shutdown();

    return 0;
}








