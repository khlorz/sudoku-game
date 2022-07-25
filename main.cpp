#include "GameWindow.h"
#include "IconsFontAwesome5.h"
#include <filesystem>
#include "ImGuiFunctions.h"
#include <Windows.h>

int main()
{
    OGLSet window_helper;
    if (!window_helper.InitGLFW("Sudoku Game", 507, 785))
        return -1;
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

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    ImFontConfig CustomFont;
    CustomFont.FontDataOwnedByAtlas = false;

    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 2;
    icons_config.OversampleV = 2;

    ImGuiIO& io = ImGui::GetIO();
    try {
        std::string dir = ((std::filesystem::current_path()).string()) + "\\Fonts\\";
        const char* fontfile_exception = "Required font not loaded! Quicksand-Medium.ttf file not found! Please make sure the required file is inside the Fonts folder.";
        const char* iconfont_exception = "Required icon font not loaded! fa-regular-400.ttf file not found! Please make sure the required file is inside the Fonts folder.";
        if (io.Fonts->AddFontFromFileTTF((dir + "Quicksand-Medium.ttf").c_str(), 15.00f) == nullptr)
            throw std::exception(fontfile_exception);
        if (io.Fonts->AddFontFromFileTTF((dir + FONT_ICON_FILE_NAME_FAR).c_str(), 15.00f, &icons_config, icons_ranges) == nullptr)
            throw std::exception(iconfont_exception);
        if(io.Fonts->AddFontFromFileTTF((dir + "Quicksand-Medium.ttf").c_str(), 18.00f) == nullptr)
            throw std::exception(fontfile_exception);
        if(io.Fonts->AddFontFromFileTTF((dir + FONT_ICON_FILE_NAME_FAR).c_str(), 18.0f, &icons_config, icons_ranges) == nullptr)
            throw std::exception(iconfont_exception);
        if(io.Fonts->AddFontFromFileTTF((dir + "Quicksand-Medium.ttf").c_str(), 22.00f) == nullptr)
            throw std::exception(fontfile_exception);
        if(io.Fonts->AddFontFromFileTTF((dir + FONT_ICON_FILE_NAME_FAR).c_str(), 22.0f, &icons_config, icons_ranges) == nullptr)
            throw std::exception(iconfont_exception);
        if(io.Fonts->AddFontDefault() == nullptr)
            throw std::exception("Internal failure!");
    }
    catch (const std::exception& e) {
        std::cout << e.what() << "\n";

        window_helper.Shutdown();
        imgui_helper.Shutdown();

        printf("Press any key to exit... Or just click the close button... ");
        std::cin.get();

        return -1;
    }

    bool show_demo = true;

    GameWindow gamewindow_helper;

    FreeConsole();

    while (!window_helper.IsWindowClosed() && !gamewindow_helper.IsWindowClosed()) {
        glfwPollEvents();
        imgui_helper.ImGuiNewFrame();

        //ImGui::ShowDemoWindow();

        gamewindow_helper.RenderWindow();

        ImGuiOGL_Render(main_window_object);
    }

    imgui_helper.Shutdown();
    window_helper.Shutdown();

    return 0;
}








