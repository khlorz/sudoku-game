#include "GameWindow.h"
#include "IconsFontAwesome5.h"
#include "imgui_internal.h"
#include <fstream>

//------------------------------------------------------------------
// LOCAL FUNCTIONS
//------------------------------------------------------------------

static void HelpToolTip(std::string_view str);
static void FinishTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str);
static void SolutionTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str);
static void CenterText(std::string_view str);

//-----------------------------------------------------------------------------------------------------------------------------------------------
// GameWindow CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------

GameWindow::GameWindow() :
    GameStart(false),
    WindowClose(false),
    Initialized(false),
    CheckGameState(false),
    ShouldShowSolution(false),
    ShouldAlwaysShowError(false),
    OpenFileWindow(false),
    SudokuFileSaved(true),
    OpenGameEndWindow(false),
    OpenLoadSaveFileWindow(false),
    CurrentlyOpenFile("None"),
    GameDifficulty(SudokuDifficulty_Normal),
    NewGameLoading("Sudoku Creation Loading Screen", "Spinner 1")
{
    // Initialize the sudoku tiles
    size_t tile_id = 0;
    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            if (!SudokuGameTiles[row][col].InitializeTile(tile_id))
                Initialized = false;
            ++tile_id;
        }
    }

    Initialized = true;
}

//----------------------------------------------------------
// WINDOW FUNCTIONS
//----------------------------------------------------------

void GameWindow::RenderWindow()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 9.0f);
    this->MainMenuBar();

    if (!Initialized) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    if (!ImGui::Begin("Main Body", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        ImGui::End();
        return;
    }

    this->GameEndWindow();

    this->FileWindow();

    this->LoadSaveFileWindow();

    this->GameMainWindow();

    this->Update();

    ImGui::End();
    ImGui::PopStyleVar();
}

bool GameWindow::GameMainWindow()
{
    this->RenderSudokuBoard();

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    ImGui::SameLine();
    this->GameOptions();

    return true;
}

void GameWindow::RenderSudokuBoard()
{
    constexpr ImVec2 child_window_size = ImVec2(475.0f, 470.0f);
    if (!ImGui::BeginChild("SudokuBoardWindow", child_window_size)) {
        ImGui::EndChild();
        return;
    }

    constexpr ImVec2 button_size         = ImVec2(48.50f, 48.50f);
    constexpr ImVec2 column_number_size  = ImVec2(48.50f, 24.25f);
    constexpr ImVec2 row_number_size     = ImVec2(24.25f, 48.50f);
    constexpr ImVec2 table_size          = ImVec2(475.0f, 425.0f);
    static const ImVec4 table_number_col = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginTable("SudokuTable", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit, table_size)) {
        ImGui::TableNextColumn();
        ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, table_number_col);
        ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 0.70f);
        ImGui::Button("##BlankCell", ImVec2(24.25f, 24.25f));
        for (size_t col = 0; col < 9; ++col) {
            char col_label[16];
            sprintf_s(col_label, 16, "%c##TCN", 'A' + col);
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            ImGui::TableNextColumn();
            ImGui::Button(col_label, column_number_size);
            ImGui::PopFont();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::EndDisabled();

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
        ImGui::BeginDisabled(!GameStart);
        for (size_t row = 0; row < 9; ++row) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::BeginDisabled(); {
                char row_label[16];
                sprintf_s(row_label, 16, "%zu##TRN", row + 1);
                ImGui::PushStyleColor(ImGuiCol_Button, table_number_col);
                ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 0.70f);
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                ImGui::Button(row_label, row_number_size);
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                ImGui::PopFont();
            } ImGui::EndDisabled();

            for (size_t col = 0; col < 9; ++col) {
                ImGui::TableNextColumn();
                if (SudokuGameTiles[row][col].RenderButton(SudokuContext, row, col, ShouldAlwaysShowError))
                    CheckGameState = true;
            }
        }
        ImGui::EndDisabled();

        ImGui::EndTable();
        ImGui::PopFont();
        ImGui::PopStyleVar(4);

        
        {   // Render a somewhat thick line for the table
            constexpr ImVec2 row_split1_pos    = ImVec2(9.50f   + 26.0f, 175.0f  + 25.0f);
            constexpr ImVec2 row_split1_sz     = ImVec2(457.0f  + 26.0f, 177.0f  + 25.0f);
            constexpr ImVec2 row_split2_pos    = ImVec2(9.50f   + 26.0f, 322.0f  + 25.0f);
            constexpr ImVec2 row_split2_sz     = ImVec2(457.0f  + 26.0f, 324.0f  + 25.0f);
            constexpr ImVec2 column_split1_pos = ImVec2(157.75f + 26.0f, 28.50f  + 25.0f);
            constexpr ImVec2 column_split1_sz  = ImVec2(159.75f + 26.0f, 470.50f + 25.0f);
            constexpr ImVec2 column_split2_pos = ImVec2(307.75f + 26.0f, 28.50f  + 25.0f);
            constexpr ImVec2 column_split2_sz  = ImVec2(309.75f + 26.0f, 470.50f + 25.0f);
            static const ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1.0f));

            ImDrawList* drawlist = ImGui::GetWindowDrawList();
            drawlist->AddRectFilled(column_split1_pos, column_split1_sz, col);
            drawlist->AddRectFilled(column_split2_pos, column_split2_sz, col);
            drawlist->AddRectFilled(row_split1_pos, row_split1_sz, col);
            drawlist->AddRectFilled(row_split2_pos, row_split2_sz, col);
        }
    }

    ImGui::EndChild();
}

void GameWindow::GameEndWindow()
{
    if (!OpenGameEndWindow)
        return;

    static std::string finish_time_note;
    static std::string solution_time_note;
    constexpr ImVec2 modal_size = ImVec2(350.0f, 200.0f);
    static bool init_modal_once = true;
    if (init_modal_once) {
        init_modal_once = false;

        // Initialize important parameters for the modal window
        ImGui::OpenPopup("Puzzle Finished!##GameEnd");
        const ImVec2& center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(modal_size, ImGuiCond_Appearing);

        // Initialize other objects that could be expensive doing in a loop
        FinishTimeNote(SudokuContext.GetBoardDifficulty(), TimeElapsed, finish_time_note);
        SolutionTimeNote(SudokuContext.GetBoardDifficulty(), ShowSolutionTotalTime, solution_time_note);
    }

    if (!ImGui::BeginPopupModal("Puzzle Finished!##GameEnd", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        OpenGameEndWindow = false;
        init_modal_once = true;
        return;
    }

    CenterText("Congratulations! You managed to finish the sudoku puzzle!");

    ImGui::Separator();
    CenterText("Time taken to finish the puzzle:");
    char finish_time[64];
    sprintf_s(finish_time, 64, "%zu hours : %zu minutes : %0.2f seconds", TimeElapsed.Hours, TimeElapsed.Minutes, TimeElapsed.Seconds);
    CenterText(finish_time);
    CenterText(finish_time_note);

    ImGui::Separator();
    CenterText("Total time taken to look at the solution:");
    char solution_look_time[64];
    sprintf_s(solution_look_time, 64, "%zu hours : %zu minutes : %0.2f seconds", ShowSolutionTotalTime.Hours, ShowSolutionTotalTime.Minutes, ShowSolutionTotalTime.Seconds);
    CenterText(solution_look_time);
    CenterText(solution_time_note);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 40.0f) * 0.50f);
    constexpr ImVec2 button_size = ImVec2(40.0f, 20.0f);
    if (ImGui::Button("Close", button_size)) {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void GameWindow::MainMenuBar()
{
    if (!ImGui::BeginMainMenuBar()) {
        ImGui::EndMainMenuBar();
        return;
    }

    if (ImGui::BeginMenu("Game")) {
        //if (ImGui::MenuItem("Change Game Parameters", NULL, false, GameStart && !GameEnd)) {
        //    GameStart = false;
        //}
        //if (ImGui::MenuItem("Restart", "Ctrl + R", false, GameStart && !GameEnd)) {
        //    RestartGame();
        //}
        //ImGui::MenuItem("Game Log", "Ctrl + G", &OpenGameLogs);
        //ImGui::MenuItem("Options", "Ctrl + O", &OpenOptions);
        //ImGui::MenuItem("Help", "Ctrl + H", &OpenHelp);
        ImGui::MenuItem("Exit", "Alt + F4", &WindowClose);
        ImGui::EndMenu();
    }

//#ifdef _DEBUG
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %0.2f", io.Framerate);
//#endif // _DEBUG

    ImGui::EndMainMenuBar();
}

void GameWindow::FileWindow()
{
    if (!OpenFileWindow)
        return;

    static bool reset_scroll_pos = true;
    static bool init_modal_once = true;
    static ImGuiTextFilter file_filter;
    static int current_file = -1;
    static std::vector<SaveFile> saved_puzzles;
    constexpr const char* directory_name = "sudoku boards";

    auto refresh_file_list = [&]() {
        if (!std::filesystem::exists(directory_name))
            std::filesystem::create_directory(directory_name);
        else if (!std::filesystem::is_directory(directory_name)) {
            std::filesystem::remove(directory_name);
            std::filesystem::create_directory(directory_name);
        }

        saved_puzzles.clear();
        for (auto& path : std::filesystem::directory_iterator(directory_name)) {
            if (path.is_directory())
                continue;

            saved_puzzles.push_back(path);
            const auto& file_time = path.last_write_time();
            std::time_t converted_time = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(file_time));
            saved_puzzles.back().DateTime = *localtime(&converted_time);
        }
    };

    if (init_modal_once) {
        init_modal_once = false;

        // Initialize important parameters for the modal window
        constexpr ImVec2 modal_size = ImVec2(425.0f, 337.0f);
        const ImVec2& center = ImGui::GetMainViewport()->GetCenter();
        ImGui::OpenPopup("Load Sudoku File##GameStart");
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(modal_size, ImGuiCond_Appearing);

        // Initialize other objects that could be expensive doing in a loop
        refresh_file_list();
    }

    if (!ImGui::BeginPopupModal("Load Sudoku File##GameStart", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        file_filter.Clear();
        current_file     = -1;
        init_modal_once  = true;
        reset_scroll_pos = true;
        OpenFileWindow   = false;
        return;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Find File:");
    ImGui::SameLine(0.0f, 2.50f);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    file_filter.Draw("##FileFilter");

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.00f, 6.00f));
    if (ImGui::BeginTable("FilesTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, 250.0f))) {
        if (reset_scroll_pos) {
            ImGui::SetScrollY(0.0f);
            reset_scroll_pos = false;
        }

        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("File Name##FT",     ImGuiTableColumnFlags_None, 225.0f);
        ImGui::TableSetupColumn("Date Modified##FT", ImGuiTableColumnFlags_None, 125.0f);
        ImGui::TableHeadersRow();

        for (int idx = 0; idx < saved_puzzles.size(); ++idx) {
            std::string_view current_filename(saved_puzzles[idx].Directory.c_str() + 14);
            if (!file_filter.PassFilter(current_filename.data()))
                continue;

            ImGui::TableNextRow();
            const bool selected = idx == current_file;

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(ICON_FA_FILE);
            ImGui::SameLine();
            if (ImGui::Selectable(current_filename.data(), selected, ImGuiSelectableFlags_SpanAllColumns))
                current_file = selected ? -1 : idx;

            ImGui::TableNextColumn();
            const auto& file_time = saved_puzzles[idx].DateTime;
            ImGui::Text("%.2d/%.2d/%.2d", file_time.tm_mday, file_time.tm_mon, file_time.tm_year + 1900);
            ImGui::SameLine(72.0f);
            ImGui::Text("%.2d:%.2d:%.2d", file_time.tm_hour, file_time.tm_min, file_time.tm_sec);
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    ImGui::BeginDisabled(current_file < 0);
    if (ImGui::Button("Open File", ImVec2(75.0f, 0))) {
        if (this->CreateNewGame(saved_puzzles[current_file].Directory))
            ImGui::CloseCurrentPopup();
        else {
            ImGui::OpenPopup("Invalid File!##ErrorWindow");
            ImGui::SetNextWindowSize(ImVec2(460.0f, 115.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }
    }
    ImGui::EndDisabled();

    if (ImGui::BeginPopupModal("Invalid File!##ErrorWindow", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        CenterText("The file you have opened is invalid or non-existent!");
        CenterText("Make sure the number of characters is equal to 81.");
        CenterText("Also make sure there are no spaces and unneccessary characters in the file.");

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 50.0f) * 0.50f);
        if (ImGui::Button("Close##ErrorCloseFOW", ImVec2(50.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        if (ImGui::IsKeyPressed(525, false) || ImGui::IsKeyPressed(526, false))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Help", ImVec2(75.0f, 0))) {
        ImGui::OpenPopup("Help##FOW");
        ImGui::SetNextWindowSize(ImVec2(460.0f, 115.0f), ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Help##FOW", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::TextUnformatted("Sudoku files should be in \"sudoku_boards\" folder.");
        ImGui::TextUnformatted("Sudoku files should contain only numerical characters and 81 characters long.");
        ImGui::TextUnformatted("Sudoku files shoud be a .txt or .dat file.");

        if (ImGui::Button("Close##HelpCloseFOW", ImVec2(50.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        if (ImGui::IsKeyPressed(525, false) || ImGui::IsKeyPressed(526, false))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh##FOW", ImVec2(75.0f, 0))) {
        refresh_file_list();
        reset_scroll_pos = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(75.0f, 0)))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void GameWindow::LoadSaveFileWindow()
{
    if (!OpenLoadSaveFileWindow)
        return;

    static bool reset_scroll_pos       = true;
    static bool init_modal_once        = true;
    static bool show_only_filled_slots = false;
    static int  selected_fidx          = -1;
    static size_t selected_difficulty = 0;
    static ImGuiTextFilter file_filter;
    static std::vector<SaveFile> save_slots(100);
    static std::string window_label;
    constexpr const char* directory_name = "save files";

    static bool init_saveslots = true;
    if (init_saveslots) {
        init_saveslots = false;
        for (int idx = 0; idx < 100; ++idx) {
            save_slots[idx].Directory.clear();
            save_slots[idx].Directory += directory_name;
            save_slots[idx].Directory += "\\";
            save_slots[idx].Directory += "save ";
            save_slots[idx].Directory += std::to_string(idx + 1);
            save_slots[idx].Directory += ".bin";
        }
    }

    auto refresh_list = [&]() {
        if (!std::filesystem::exists(directory_name))
            std::filesystem::create_directory(directory_name);
        else if (!std::filesystem::is_directory(directory_name)) {
            std::filesystem::remove(directory_name);
            std::filesystem::create_directory(directory_name);
        }

        for (auto& save_slot : save_slots) {
            if (save_slot.Exists = std::filesystem::exists(save_slot.Directory)) {
                save_slot.Difficulty = sdq::utils::LoadDifficultyFromSaveFile(save_slot.Directory.data());
                const auto& file_time = std::filesystem::directory_entry(save_slot.Directory).last_write_time();
                std::time_t converted_time = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(file_time));
                localtime_s(&save_slot.DateTime, &converted_time);
            }
            else
                save_slot.Difficulty = 0;
        }
    };

    if (init_modal_once) {
        init_modal_once = false;

        // Initialize important parameters for the modal window
        constexpr ImVec2 modal_size = ImVec2(425.0f, 337.0f);
        const ImVec2& center = ImGui::GetMainViewport()->GetCenter();
        window_label = LoadAFile ? "Load Save File##GameStart" : "Save Current Progress##SaveGame";
        ImGui::OpenPopup(window_label.data());
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(modal_size, ImGuiCond_Appearing);

        // Initialize other objects that could be expensive doing in a loop
        refresh_list();
        file_filter.Clear();
        selected_fidx          = -1;
        selected_difficulty    = 0;
        reset_scroll_pos       = true;
        show_only_filled_slots = LoadAFile;
    }

    if (!ImGui::BeginPopupModal(window_label.data(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        init_modal_once = true;
        OpenLoadSaveFileWindow = false;
        return;
    }

    constexpr std::array<const char*, 5> difficulty_choices = { "All", "Easy", "Normal", "Insane", "Diabolical"};
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Filter Difficulty");
    ImGui::SameLine(0.0f, 2.50f);
    ImGui::PushItemWidth(162.0f);
    if (ImGui::BeginCombo("##difficulty_combo", difficulty_choices[selected_difficulty])) {
        for (int n = 0; n < difficulty_choices.size(); n++) {
            const bool is_selected = (selected_difficulty == n);
            if (ImGui::Selectable(difficulty_choices[n], is_selected)) {
                if (!is_selected) {
                    selected_difficulty = n;
                    selected_fidx = -1;
                }
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Show Saved Slots Only");
    ImGui::SameLine(0.0f, 2.50f);
    ImGui::Checkbox("##ShowSavedSlots", &show_only_filled_slots);

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.00f, 6.00f));
    if (ImGui::BeginTable("FilesTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, 250.0f))) {
        if (reset_scroll_pos) {
            ImGui::SetScrollY(0.0f);
            reset_scroll_pos = false;
        }

        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("Save Slot##LSF",     ImGuiTableColumnFlags_None, 175.0f);
        ImGui::TableSetupColumn("Difficulty##LSF",    ImGuiTableColumnFlags_None, 100.0f);
        ImGui::TableSetupColumn("Date Modified##LSF", ImGuiTableColumnFlags_None, 125.0f);
        ImGui::TableHeadersRow();

        for (int idx = 0; idx < save_slots.size(); ++idx) {
            std::string_view filename = std::string_view(save_slots[idx].Directory.begin() + 11, save_slots[idx].Directory.end());
            if (!file_filter.PassFilter(filename.data()))
                continue;

            if (show_only_filled_slots && !save_slots[idx].Exists)
                continue;

            if (selected_difficulty != 0 && save_slots[idx].Difficulty != selected_difficulty)
                continue;

            const bool selected = idx == selected_fidx;
            const bool empty_slot = !save_slots[idx].Exists;
            
            ImGui::BeginDisabled(empty_slot && LoadAFile);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(ICON_FA_FILE);
            ImGui::SameLine();
            if (ImGui::Selectable(filename.data(), selected, ImGuiSelectableFlags_SpanAllColumns))
                selected_fidx = selected ? -1 : idx;
            if (empty_slot) {
                ImGui::SameLine(100.0f);
                ImGui::TextUnformatted("[empty]");
            }

            ImGui::TableNextColumn();
            switch (save_slots[idx].Difficulty)
            {
            case SudokuDifficulty_Easy:       ImGui::TextUnformatted("Easy");       break;
            case SudokuDifficulty_Normal:     ImGui::TextUnformatted("Normal");     break;
            case SudokuDifficulty_Insane:     ImGui::TextUnformatted("Insane");     break;
            case SudokuDifficulty_Diabolical: ImGui::TextUnformatted("Diabolical"); break;
            default:                          ImGui::TextUnformatted("--------");       break;
            }

            ImGui::TableNextColumn();
            if (empty_slot)
                ImGui::TextUnformatted("--/--/-- -- : -- : --");
            else {
                const auto& file_time = save_slots[idx].DateTime;
                ImGui::Text("%.2d/%.2d/%.2d", file_time.tm_mday, file_time.tm_mon, file_time.tm_year + 1900);
                ImGui::SameLine(72.0f);
                ImGui::Text("%.2d:%.2d:%.2d", file_time.tm_hour, file_time.tm_min, file_time.tm_sec);
            }
            ImGui::EndDisabled();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    ImGui::BeginDisabled(selected_fidx < 0);
    if (ImGui::Button(LoadAFile ? "Open File##LSF" : "Save File##LSF", ImVec2(75.0f, 0))) {
        if (LoadAFile) {
            if (this->LoadSaveFile(save_slots[selected_fidx].Directory))
                ImGui::CloseCurrentPopup();
            else {
                ImGui::OpenPopup("Invalid File!##ErrorWindowLSF");
                ImGui::SetNextWindowSize(ImVec2(400.0f, 96.0f), ImGuiCond_Appearing);
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            }
        }
        else {
            if (save_slots[selected_fidx].Exists) {
                ImGui::OpenPopup("Overwriting Save File##ErrorWindowLSF");
                ImGui::SetNextWindowSize(ImVec2(360.0f, 96.0f), ImGuiCond_Appearing);
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            }
            else if (this->SaveProgress(save_slots[selected_fidx].Directory)) {
                refresh_list();
                selected_fidx = -1;
            }
        }
    }
    ImGui::EndDisabled();

    if (ImGui::BeginPopupModal("Overwriting Save File##ErrorWindowLSF", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        CenterText("A save file already exist on that slot!");
        CenterText("Do you want to overwrite the current save file?");

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100.0f) * 0.50f);
        if (ImGui::Button("Yes", ImVec2(50.0f, 0.0f))) {
            if (this->SaveProgress(save_slots[selected_fidx].Directory)) {
                refresh_list();
                selected_fidx = -1;
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(50.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Invalid File!##ErrorWindowLSF", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        CenterText("The file you have opened is invalid or non-existent!");
        CenterText("Make sure the file is a file you saved from this application!");

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 50.0f) * 0.50f);
        if (ImGui::Button("Close##ErrorCloseLSF", ImVec2(50.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        if (ImGui::IsKeyPressed(525, false) || ImGui::IsKeyPressed(526, false))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(selected_fidx < 0 || !save_slots[selected_fidx].Exists);
    if (ImGui::Button("Delete File", ImVec2(75.0f, 0.0f))) {
        ImGui::OpenPopup("Deleting Save File##ErrorWindowLSF");
        ImGui::SetNextWindowSize(ImVec2(360.0f, 100.0f), ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
    ImGui::EndDisabled();

    if (ImGui::BeginPopupModal("Deleting Save File##ErrorWindowLSF", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        CenterText("This action cannot be undone!");
        CenterText("Are you sure you want to delete the selected file?");

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100.0f) * 0.50f);
        if (ImGui::Button("Yes", ImVec2(50.0f, 0.0f))) {
            std::filesystem::remove(save_slots[selected_fidx].Directory);
            refresh_list();
            selected_fidx = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(50.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Help##LSF", ImVec2(75.0f, 0))) {
        ImGui::OpenPopup("Help##LSF");
        ImGui::SetNextWindowSize(ImVec2(390.0f, 96.0f), ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Help##LSF", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::TextUnformatted("Save files should be in \"save files\" folder.");
        ImGui::TextUnformatted("Save files should only be made and opened from this application.");

        if (ImGui::Button("Close##HelpCloseLSF", ImVec2(50.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        if (ImGui::IsKeyPressed(525, false) || ImGui::IsKeyPressed(526, false))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh", ImVec2(75.0f, 0))) {
        refresh_list();
        reset_scroll_pos = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel##LSF", ImVec2(75.0f, 0)))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void GameWindow::GameOptions()
{
    if (!ImGui::BeginChild("GameOptionsWindow")) {
        ImGui::EndChild();
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
//#ifdef _DEBUG
    constexpr std::array<const char*, 7> game_difficulties = { "Random", "Easy As Pie", "Normal And Average", "Insane Enjoyer", "Diabolical Chad", "Load A Sudoku Puzzle", "Load A Save File"};
//#else
    //constexpr std::array<const char*, 5> game_difficulties = { "Random", "Easy As Pie", "Normal And Average", "Insane Enjoyer", "Diabolical Chad" };
//#endif // _DEBUG
    const char* current_difficulty = game_difficulties[GameDifficulty];

    ImGui::Text("Sudoku Difficulty: %s", game_difficulties[SudokuContext.GetBoardDifficulty()]);
    ImGui::PushItemWidth(160.0f);
    if (ImGui::BeginCombo("##Sudoku Difficulty", current_difficulty)) {
        for (int n = 0; n < game_difficulties.size(); n++) {
            const bool is_selected = GameDifficulty == n;
            if (ImGui::Selectable(game_difficulties[n], is_selected))
                GameDifficulty = n;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("New Game", ImVec2(80.0f, 0.0f))) {
        if (GameDifficulty == 5)
            OpenFileWindow = true;
        else if (GameDifficulty == 6) {
            OpenLoadSaveFileWindow = true;
            LoadAFile = true;
        }
        else {
            using NewGameFromDifficulty = bool(GameWindow::*)(int);
            NewGameFuture = std::async(std::launch::async, static_cast<NewGameFromDifficulty>(&GameWindow::CreateNewGame), this, GameDifficulty);
            NewGameLoading.StartLoading(ImVec2(120.0f, 120.0f));
        }
    }

    static bool not_done = false;
    if (NewGameFuture.valid()) {
        not_done = NewGameFuture.wait_for(std::chrono::microseconds(1)) != std::future_status::ready;
        if (!not_done)
            NewGameFuture.get();
    }
    NewGameLoading.RenderLoading(not_done, 40.0f, 0.0f, 5.0f);


    static const std::string difficulty_help =
        "Random - Generates a puzzle of random difficulty puzzle.\n\n"
        "Easy As Pie - Generates an easy puzzle. Best for beginners.\n\n"
        "Normal And Average - Generates a normal puzzle. Best for passing time.\n\n"
        "Insane Enjoyer - Generates a hard puzzle. Best for passionate problem solvers.\n\n"
        "Diabolical Chad - Generates a really sadistic puzzle. Best for the chad of chads.";
    //"Custom Board - Create your own sudoku puzzle!";
    ImGui::SameLine();
    HelpToolTip(difficulty_help);

//#ifdef _DEBUG
//    if (GameDifficulty == SudokuDifficulty_Custom) {
//        static int number_of_remaining_tiles = 30;
//        ImGui::Text("Number Of Remaining Tiles: %d", number_of_remaining_tiles);
//        ImGui::SliderInt("RemainingTilesSlider", &number_of_remaining_tiles, 17, 50, "", ImGuiSliderFlags_AlwaysClamp);
//    }
//#endif // _DEBUG

    ImGui::BeginDisabled(!GameStart);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.00f, 4.00f));
    ImGui::BeginGroup(); {
        auto combo_lambda = [](const char* label, const std::array<const char*, 9>& choices, int& current_choice) {
            if (ImGui::BeginCombo(label, choices[current_choice])) {
                for (int n = 0; n < choices.size(); n++) {
                    const bool is_selected = (current_choice == n);
                    if (ImGui::Selectable(choices[n], is_selected)) {
                        current_choice = n;
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        };

        constexpr float combo_width         = 150.0f;
        constexpr float text_widget_spacing = 88.0f;
        constexpr std::array<const char*, 9> cell_choices = { "Cell 1: A1 - C3", "Cell 2: D1 - F3", "Cell 3: G1 - I3", "Cell 4: A4 - C6", "Cell 5: D4 - F6", "Cell 6: G4 - I6", "Cell 7: A7 - C9", "Cell 8: D7 - F9", "Cell 9: G7 - I9" };
        static int cell_clear_num = 0;
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Clear Cell: ");
        ImGui::SameLine(text_widget_spacing);
        ImGui::PushItemWidth(combo_width);
        combo_lambda("##ClearCellCombo", cell_choices, cell_clear_num);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##CellClear", ImGuiDir_Right)) {
            int actual_cell = cell_clear_num;
            int min_row = (actual_cell / 3) * 3;
            int max_row = min_row + 3;
            int min_col = (actual_cell % 3) * 3;
            int max_col = min_col + 3;
            for (int row = min_row; row < max_row; ++row) {
                for (int col = min_col; col < max_col; ++col) {
                    if (!SudokuGameTiles[row][col].IsTileFilled())
                        continue;

                    SudokuGameTiles[row][col].ResetTile();
                    if (SudokuGameTiles[row][col].IsPuzzleTile())
                        SudokuContext.SetTile(row, col, 0);
                }
            }
        }

        constexpr std::array<const char*, 9> row_choices = { "Row 1: A1 - I1", "Row 2: A2 - I2", "Row 3: A3 - I3", "Row 4: A4 - I4", "Row 5: A5 - I5", "Row 6: A6 - I6", "Row 7: A7 - I7", "Row 8: A8 - I8", "Row 9: A9 - I9" };
        static int row_clear_num = 0;
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Clear Row: ");
        ImGui::SameLine(text_widget_spacing);
        ImGui::PushItemWidth(combo_width);
        combo_lambda("##RowClearCombo", row_choices, row_clear_num);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##RowClear", ImGuiDir_Right)) {
            int actual_row = row_clear_num;
            for (int col = 0; col < 9; ++col) {
                if (!SudokuGameTiles[actual_row][col].IsTileFilled())
                    continue;

                SudokuGameTiles[actual_row][col].ResetTile();
                if (SudokuGameTiles[actual_row][col].IsPuzzleTile())
                    SudokuContext.SetTile(actual_row, col, 0);
            }
            printf("done\n");
        }

        constexpr std::array<const char*, 9> column_choices = { "Column 1: A1 - A9", "Column 2: B1 - B9", "Column 3: C1 - C9", "Column 4: D1 - D9", "Column 5: E1 - E9", "Column 6: F1 - F9", "Column 7: G1 - G9", "Column 8: H1 - H9", "Column 9: I1 - I9" };
        static int column_clear_num = 0;
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Clear Column: ");
        ImGui::SameLine(text_widget_spacing);
        ImGui::PushItemWidth(combo_width);
        combo_lambda("##ColumnClearCombo", column_choices, column_clear_num);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##ColClear", ImGuiDir_Right)) {
            int actual_col = column_clear_num;
            for (int row = 0; row < 9; ++row) {
                if (!SudokuGameTiles[row][actual_col].IsTileFilled())
                    continue;

                SudokuGameTiles[row][actual_col].ResetTile();
                if (SudokuGameTiles[row][actual_col].IsPuzzleTile())
                    SudokuContext.SetTile(row, actual_col, 0);
            }
        }
    } ImGui::EndGroup();
    ImGui::PopStyleVar();

    ImGui::Checkbox("Show Errors", &ShouldAlwaysShowError);
    ImGui::SameLine();
    static const std::string showerror_help =
        "Shows all the current and upcoming errors. "
        "Errored tiles are red but will not be replaced by the correct number!";
    HelpToolTip(showerror_help);
    ImGui::SameLine();
    if (ImGui::Checkbox("Show Solution", &ShouldShowSolution))
        this->SetShowSolution();
    ImGui::SameLine();
    static const std::string checksolution_help =
        "Shows the puzzle's solution. "
        "Errored tiles will be in red and will be replaced by the correct number!\n"
        "This is kind of cheating so... Do not use this often.";
    HelpToolTip(checksolution_help);

#ifdef _DEBUG
    if (ImGui::Button("Complete the puzzle!")) {
        SudokuContext.FillPuzzleBoard();
        CheckGameState = true;
    }
#endif
    if (ImGui::Button("Save Progress##GOW", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 0.0f))) {
        OpenLoadSaveFileWindow = true;
        LoadAFile = false;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(350.0f);
        ImGui::TextUnformatted("Save the current progress of the puzzle you are solving");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    ImGui::EndDisabled();

    {
        static std::string filepath;
        ImGui::BeginDisabled(SudokuFileSaved);
        ImGui::SameLine();
        if (ImGui::Button("Save Puzzle##GOW", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
            static std::string puzzle_filename;
            constexpr const char* folder_name = "sudoku boards";
            constexpr const char* file_extension = ".txt";
            SudokuDifficulty difficulty = SudokuContext.GetBoardDifficulty();
            switch (difficulty)
            {
            case SudokuDifficulty_Easy:       puzzle_filename = "easy";       break;
            case SudokuDifficulty_Normal:     puzzle_filename = "normal";     break;
            case SudokuDifficulty_Insane:     puzzle_filename = "insane";     break;
            case SudokuDifficulty_Diabolical: puzzle_filename = "diabolical"; break;
            default:                          puzzle_filename = "random";     break;
            }

            if (!std::filesystem::exists(folder_name))
                std::filesystem::create_directory(folder_name);
            else if (!std::filesystem::is_directory(folder_name)) {
                std::filesystem::remove(folder_name);
                std::filesystem::create_directory(folder_name);
            }

            int file_number = 1;
            do {
                filepath.clear();
                filepath += folder_name;
                filepath += "\\";
                filepath += puzzle_filename;
                filepath += std::to_string(file_number++);
                filepath += file_extension;
            } while (std::filesystem::exists(filepath));

            if (sdq::utils::CreateSudokuFile(*SudokuContext.GetPuzzleBoard(), filepath.c_str())) {
                SudokuFileSaved = true;
                ImGui::OpenPopup("Puzzle Successfully Saved##GOW");
                ImGui::SetNextWindowSize(ImVec2(400.0f, 100.0f), ImGuiCond_Appearing);
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(350.0f);
            ImGui::TextUnformatted(GameStart ? "Create a sudoku file from the current puzzle you are solving." :
                                               "Create a sudoku file from the puzzle you solved.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        ImGui::EndDisabled();

        if (ImGui::BeginPopupModal("Puzzle Successfully Saved##GOW", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            CenterText("Puzzle file successfully saved!");
            CenterText("Saved as:");
            CenterText(std::string_view(filepath.begin() + 14, filepath.end()));

            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 50.0f) * 0.50f);
            if (ImGui::Button("Close", ImVec2(50.0f, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    const float bottom_pos = ImGui::GetWindowSize().y - ImGui::GetTextLineHeightWithSpacing();
    ImGui::SetCursorPosY(bottom_pos);
    ImGui::Text("Currently Opened File: %s", CurrentlyOpenFile.c_str());

    ImGui::EndChild();
}

//----------------------------------------------------------
// ACTION FUNCTIONS
//----------------------------------------------------------

bool GameWindow::IsWindowClosed()
{
    return WindowClose;
}

bool GameWindow::CreateNewGame(SudokuDifficulty difficulty)
{
    std::lock_guard func_guard(NewGameMutex);
    if (!SudokuContext.CreateSudoku(difficulty))
        return false;

    this->StopOngoingGame();
    GameStart = true;
    SudokuFileSaved = false;
    CurrentlyOpenFile = "None";
    this->SetShowSolution();
    this->SetSudokuTilesForNewGame();

    return true;
}

bool GameWindow::CreateNewGame(const std::string& filepath)
{
    const auto& input_sudoku_board = sdq::utils::OpenSudokuFile(filepath.c_str());
    if (!input_sudoku_board.has_value())
        return false;

    if (!SudokuContext.CreateSudoku(input_sudoku_board.value()))
        return false;

    this->StopOngoingGame();
    GameStart = true;
    SudokuFileSaved = true;
    CurrentlyOpenFile = std::string_view(filepath.begin() + 14, filepath.end());
    this->SetShowSolution();
    this->SetSudokuTilesForNewGame();

    return true;
}

void GameWindow::StopOngoingGame()
{
    GameStart             = false;
    ShouldAlwaysShowError = false;
    ShouldAlwaysShowError = false;
    TimeElapsed.Clear();
    ShowSolutionTotalTime.Clear();
}

void GameWindow::SetSudokuTilesForNewGame()
{
    const auto& puzzle_board = SudokuContext.GetPuzzleBoard();
    const auto& solution_board = SudokuContext.GetSolutionBoard();

    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            SudokuGameTiles[row][col].SetTilePuzzleNumber(puzzle_board->GetTile(row, col).TileNumber,
                                                          solution_board->GetTile(row, col).TileNumber,
                                                          puzzle_board->GetTile(row, col).TileNumber == 0);
        }
    }
}

void GameWindow::SetSudokuTileFromSaveFile()
{
    const auto& puzzle_board   = SudokuContext.GetPuzzleBoard();
    const auto& solution_board = SudokuContext.GetSolutionBoard();

    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            SudokuGameTiles[row][col].SetTilePuzzleNumber(puzzle_board->GetTile(row, col).TileNumber,
                                                          solution_board->GetTile(row, col).TileNumber,
                                                          std::find_if(puzzle_board->PuzzleTiles.begin(), 
                                                                       puzzle_board->PuzzleTiles.end(), 
                                                                       [&](const sdq::BoardTile* tile){ return tile->Row == row && tile->Column == col; } ) != puzzle_board->PuzzleTiles.end());
        }
    }
}

void GameWindow::SetShowSolution()
{
    for (size_t row = 0; row < 9; ++row) 
        for (size_t col = 0; col < 9; ++col) 
            SudokuGameTiles[row][col].SetAsSolutionTile(ShouldShowSolution);
}

void GameWindow::RecheckTiles()
{
    for (size_t row = 0; row < 9; row++)
        for (size_t col = 0; col < 9; ++col)
            SudokuGameTiles[row][col].RecheckError(SudokuContext, row, col);
}

bool GameWindow::SaveProgress(const std::string& filepath)
{
    return sdq::utils::SaveSudokuProgress(SudokuContext, filepath.data());
}

void GameWindow::Update()
{
    ImGuiIO io = ImGui::GetIO();

    if (GameStart)
        TimeElapsed += io.DeltaTime;

    if (ShouldShowSolution)
        ShowSolutionTotalTime += io.DeltaTime;

    if (CheckGameState) {
        if (SudokuContext.CheckPuzzleState()) {
            GameStart             = false;
            OpenGameEndWindow     = true;
            ShouldShowSolution    = false;
            ShouldAlwaysShowError = false;
        }
        else {
            RecheckTiles();
        }
        CheckGameState = false;
    }
}

bool GameWindow::LoadSaveFile(const std::string& filepath)
{
    if(!sdq::utils::LoadSudokuProgress(SudokuContext, filepath.data()))
        return false;
    
    this->StopOngoingGame();
    GameStart         = true;
    SudokuFileSaved   = false;
    CurrentlyOpenFile = std::string_view(filepath.begin() + 11, filepath.end());
    this->SetShowSolution();
    this->SetSudokuTileFromSaveFile();
    this->RecheckTiles();
    return true;
}



//-----------------------------------------------------------------------------------------------------------------------------------------------
// SudokuTile CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------

SudokuTile::SudokuTile() :
    Initialized(false),
    ErrorTile(false),
    PuzzleTile(true),
    ShowAsSolution(false),
    SolutionNumber(0),
    TileNumber(0), 
    TileID(0)
{}

SudokuTile::SudokuTile(int item_num) :
    TileNumber(0), TileID(item_num), SolutionNumber(0), Initialized(false), ErrorTile(false), PuzzleTile(false), ShowAsSolution(false)
{
    sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID);
    sprintf_s(ContextPopUpLabel, CharBufferSize, "##CPU%d", TileID);
    sprintf_s(InputIntLabel, CharBufferSize, "##II%d", TileID);
}

bool SudokuTile::InitializeTile(int item_num)
{
    if (Initialized)
        return true;

    TileID       = item_num;
    TileNumber   = 0;
    ErrorTile    = false;
    PuzzleTile   = false;
    // A series of early out statements in case the sprintf_s fails
    bool success = TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID) > 0 : sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) > 0;
    if (!success) {
        Initialized = false;
        return false;
    }
    if (sprintf_s(ContextPopUpLabel, CharBufferSize, "##CPU%d", TileID) < 0) {
        Initialized = false;
        return false;
    }
    if (sprintf_s(InputIntLabel, CharBufferSize, "##II%d", TileID) < 0) {
        Initialized = false;
        return false;
    }

    Initialized = true;
    return true;
}

void SudokuTile::SetTilePuzzleNumber(uint16_t t_number, uint16_t solution_number, bool is_puzzle)
{
    TileNumber     = t_number;
    ErrorTile      = false;
    PuzzleTile     = is_puzzle; 
    SolutionNumber = solution_number;
    TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID) > 0 :
                      sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) > 0;
}

void SudokuTile::ResetTile()
{
    // Early out if its not a puzzle number so it won't be changed
    if (!PuzzleTile)
        return;

    TileNumber = 0;
    ErrorTile  = false;
    sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID);
}

bool SudokuTile::RenderButton(sdq::GameContext& sudoku_context, uint16_t row, uint16_t col, bool error_override)
{
    bool pressed = false;
    constexpr ImU32 error_button_col        = 2600468659;
    constexpr ImU32 error_buttonhovered_col = 2721396170;
    constexpr ImU32 error_buttonactive_col  = 2267161319;
    const bool error_tile                   = (ErrorTile && error_override) || (ShowAsSolution && TileNumber != SolutionNumber);
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, ShowAsSolution ? 0.90f : 0.80f);
    ImGui::PushStyleColor(ImGuiCol_Button       , error_tile ? error_button_col        : ImGui::GetColorU32(ImGuiCol_Button));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, error_tile ? error_buttonhovered_col : ImGui::GetColorU32(ImGuiCol_ButtonHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive , error_tile ? error_buttonactive_col  : ImGui::GetColorU32(ImGuiCol_ButtonActive));
    ImGui::BeginDisabled(!PuzzleTile || ShowAsSolution);
    ImGui::Button(ButtonLabel, ButtonSize);
    ImGui::PopStyleColor(3);
    if (ImGui::BeginPopupContextItem(ContextPopUpLabel, 0)) {
        ImGui::PushItemWidth(25.0f);
        ImGui::SetKeyboardFocusHere();
        ImGui::InputInt(InputIntLabel, &TileNumber, 0, 0);
        if (ImGui::IsKeyPressed(526, false)) {
            ImGui::CloseCurrentPopup();
        }
        else if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsKeyPressed(525, false)) {
            if (TileNumber < 0) { TileNumber = 0; }
            if (TileNumber > 9) { TileNumber = 9; }
            TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID) < 0 : sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) < 0;
            ErrorTile = !sudoku_context.SetTile(row, col, TileNumber);
            pressed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::EndDisabled();
    ImGui::PopStyleVar();

    return pressed && TileNumber != 0;
}

void SudokuTile::SetAsSolutionTile(bool enable)
{
    // Early out in case the tile is a puzzle tile since it's already the correct number
    if (!PuzzleTile)
        return;

    if (!enable)
        TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID) : sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) ;
    else
        sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", SolutionNumber, TileID);

    ShowAsSolution = enable;
}

bool SudokuTile::IsPuzzleTile()
{
    return PuzzleTile;
}

bool SudokuTile::IsTileFilled()
{
    return TileNumber != 0;
}

void SudokuTile::RecheckError(sdq::GameContext& sudoku_context, uint16_t row, uint16_t col)
{
    if (!PuzzleTile)
        return;

    ErrorTile = !sudoku_context.IsValidTile(row, col);
}


//------------------------------------------------------------------
// LOCAL FUNCTIONS' DEFINITIONS
//------------------------------------------------------------------

static void HelpToolTip(std::string_view str)
{
    ImGui::TextUnformatted(ICON_FA_QUESTION_CIRCLE);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(350.0f);
        ImGui::TextUnformatted(str.data());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void FinishTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str)
{
    switch (difficulty)
    {
    case SudokuDifficulty_Easy:
        if (time_elapse.Minutes >= 0 && time_elapse.Minutes < 5 && time_elapse.Hours == 0) {
            str = "You are ready for the next battle!";
        }
        else if (time_elapse.Minutes >= 5 && time_elapse.Minutes < 20 && time_elapse.Hours == 0) {
            str = "You are doing a good job!";
        }
        else if (time_elapse.Minutes >= 20 && time_elapse.Minutes < 40 && time_elapse.Hours == 0) {
            str = "More practice will make you better!";
        }
        else {
            str = "Never give up...";
        }
        return;
    case SudokuDifficulty_Normal:
        if (time_elapse.Minutes >= 0 && time_elapse.Minutes < 5 && time_elapse.Hours == 0) {
            str = "You are ready for the next battle!";
        }
        else if (time_elapse.Minutes >= 5 && time_elapse.Minutes < 20 && time_elapse.Hours == 0) {
            str = "You are already on par with the average sudoku solvers";
        }
        else if (time_elapse.Minutes >= 20 && time_elapse.Minutes < 40 && time_elapse.Hours == 0) {
            str = "You can do it better next time! Just try and try!";
        }
        else {
            str = "Maybe you should first try easy mode...";
        }
        return;
    case SudokuDifficulty_Insane:
        if (time_elapse.Minutes >= 0 && time_elapse.Minutes < 5 && time_elapse.Hours == 0) {
            str = "You are ready for the next battle!";
        }
        else if (time_elapse.Minutes >= 5 && time_elapse.Minutes < 20 && time_elapse.Hours == 0) {
            str = "You really are working your guts off for sudoku huh? Great!";
        }
        else if (time_elapse.Minutes >= 20 && time_elapse.Minutes < 40 && time_elapse.Hours == 0) {
            str = "Must be a difficulty shock or something...";
        }
        else {
            str = "\"Know the difficulty before you solve it\" - Sun Tzu, maybe";
        }
        return;
    case SudokuDifficulty_Diabolical:
        if (time_elapse.Minutes >= 0 && time_elapse.Minutes < 10 && time_elapse.Hours == 0) {
            str = "Here's your crown, king.";
        }
        else if (time_elapse.Minutes >= 10 && time_elapse.Minutes < 20 && time_elapse.Hours == 0) {
            str = "You still solve it despite the harsh difficulty. That has merits on its own!";
        }
        else if (time_elapse.Minutes >= 20 && time_elapse.Minutes < 40 && time_elapse.Hours == 0) {
            str = "Kinda below average for someone who wanted to solve a difficulty for chads.";
        }
        else {
            str = "Well, I guess this difficulty was just not for you. It's you, not the difficulty.";
        }
        return;
    default:
        assert(false);
        break;
    }
}

static void SolutionTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str)
{
    if (time_elapse.Minutes == 0 && time_elapse.Seconds == 0.0f && time_elapse.Hours == 0) {
        str = "Niceeeeeeeeeeeeee!";
    }
    else if (time_elapse.Seconds > 0.0f && time_elapse.Seconds < 5.0f && time_elapse.Minutes == 0 && time_elapse.Hours == 0) {
        str = "Hmmmmmmmmmmmm. Forgivable";
    }
    else if (time_elapse.Seconds >= 5.0f && time_elapse.Seconds < 10.0f && time_elapse.Minutes == 0 && time_elapse.Hours == 0) {
        str = "You had a bad time. It's barely forgivable though.";
    }
    else if (time_elapse.Seconds >= 10.0f && time_elapse.Minutes < 1 && time_elapse.Hours == 0) {
        str = "You must be having a really hard time solving the puzzle";
    }
    else if (time_elapse.Minutes >= 1 && time_elapse.Minutes < 5 && time_elapse.Hours == 0) {
        str = "Well, maybe you're just new or maybe you just can solve the puzzle.";
    }
    else {
        str = "There must be something wrong here. It took you more than 5 minutes to look at the solution before solving the puzzle!.";
    }
}

static void CenterText(std::string_view str)
{
    const auto& window_width = ImGui::GetWindowSize().x;
    const auto& text_width = ImGui::CalcTextSize(str.data()).x;

    ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
    ImGui::TextUnformatted(str.data());
}





