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
// A simple combo wrapper for arrays
template<size_t S>
static void SimpleComboWrapper(const char* label, const std::array<const char*, S>& choices, int& current_choice);

static int PencilmarkButton(const char* label, const ImVec2& sz, const std::bitset<9>& pencilmark);

//-----------------------------------------------------------------------------------------------------------------------------------------------
// GameWindow CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------

GameWindow::GameWindow() :
    GameStart(false),
    GamePaused(false),
    WindowClose(false),
    Initialized(false),
    CheckGameState(false),
    ShowSolution(false),
    ShowError(false),
    OpenLoadSudokuWindow(false),
    SudokuFileSaved(true),
    OpenGameEndWindow(false),
    OpenLoadSaveFileWindow(false),
    ShowLoadingScreen(false),
    StartLoadingScreen(false),
    ShowPencilmarks(false),
    NewGameResult(std::nullopt),
    CurrentlyOpenFile("None"),
    GameDifficulty(SudokuDifficulty_Normal),
    NewGameLoading("Sudoku Creation Loading Screen", "Spinner 1")
{
    // Initialize the sudoku tiles
    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            if (!SudokuGameTiles[row][col].InitializeTile(SudokuContext.GetPuzzleBoard()->GetTile(row, col), SudokuContext.GetSolutionBoard()->GetTile(row, col)))
                Initialized = false;
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

    this->LoadSudokuFileWindow();

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
            sprintf_s(col_label, 16, "%zu##TCN", col + 1);
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
        ImGui::BeginDisabled(!GameStart || GamePaused);
        for (size_t row = 0; row < 9; ++row) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::BeginDisabled(); {
                char row_label[16];
                sprintf_s(row_label, 16, "%c##TRN", 'A' + row);
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
                if (GamePaused ? ImGui::Button("##EmptyButton", SudokuTile::ButtonSize) : SudokuGameTiles[row][col].RenderButton(SudokuContext, row, col, ShowError)) {
                    SudokuGameTiles[row][col].UpdateTileNumber(ShowPencilmarks && !SudokuContext.GetPuzzleBoard()->GetTile(row, col).IsTileFilled() ? TileState_Pencilmark : TileState_Normal);
                    CheckGameState = true;
                }
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
    if (ImGui::Button("Close", button_size))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void GameWindow::MainMenuBar()
{
    if (!ImGui::BeginMainMenuBar()) {
        ImGui::EndMainMenuBar();
        return;
    }

    if (ImGui::BeginMenu("Game")) {
        ImGui::MenuItem("Exit", "Alt + F4", &WindowClose);
        ImGui::EndMenu();
    }

#ifdef _DEBUG
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %0.2f", io.Framerate);
#endif // _DEBUG

    ImGui::EndMainMenuBar();
}

void GameWindow::LoadSudokuFileWindow()
{
    if (!OpenLoadSudokuWindow)
        return;

    static bool reset_scroll_pos = true;
    static bool init_modal_once = true;
    static ImGuiTextFilter file_filter;
    static int selected_fidx = -1;
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
        selected_fidx     = -1;
        init_modal_once  = true;
        reset_scroll_pos = true;
        OpenLoadSudokuWindow   = false;
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
            const bool selected = idx == selected_fidx;

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(ICON_FA_FILE);
            ImGui::SameLine();
            if (ImGui::Selectable(current_filename.data(), selected, ImGuiSelectableFlags_SpanAllColumns))
                selected_fidx = selected ? -1 : idx;

            ImGui::TableNextColumn();
            const auto& file_time = saved_puzzles[idx].DateTime;
            ImGui::Text("%.2d/%.2d/%.2d", file_time.tm_mday, file_time.tm_mon, file_time.tm_year + 1900);
            ImGui::SameLine(72.0f);
            ImGui::Text("%.2d:%.2d:%.2d", file_time.tm_hour, file_time.tm_min, file_time.tm_sec);
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    ImGui::BeginDisabled(selected_fidx < 0);
    if (ImGui::Button("Open File", ImVec2(75.0f, 0))) {
        using NewGameFromSave = bool (GameWindow::*)(const std::string&);
        NewGameFuture = std::async(std::launch::async, static_cast<NewGameFromSave>(&GameWindow::CreateNewGame), this, saved_puzzles[selected_fidx].Directory);
        StartNewGameLoadingScreen();
    }
    ImGui::EndDisabled();

    this->CheckNewGameProgress();
    this->RenderNewGameLoadingScreen();

    if (NewGameResult.has_value()) {
        if (NewGameResult.value())
            ImGui::CloseCurrentPopup();
        else {
            ImGui::OpenPopup("Invalid File!##ErrorWindow");
            ImGui::SetNextWindowSize(ImVec2(460.0f, 115.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }
        NewGameResult.reset();
    }

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
            save_slot.Exists = std::filesystem::exists(save_slot.Directory);
            if (!save_slot.Exists) {
                save_slot.Difficulty = 0;
                continue;
            }

            save_slot.Difficulty = sdq::Instance::LoadDifficultyFromSaveFile(save_slot.Directory.data());
            const auto& file_time = std::filesystem::directory_entry(save_slot.Directory).last_write_time();
            std::time_t converted_time = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(file_time));
            localtime_s(&save_slot.DateTime, &converted_time);
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
            default:                          ImGui::TextUnformatted("--------");   break;
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
            using NewGameFromSave = bool (GameWindow::*)(const std::string&);
            NewGameFuture = std::async(std::launch::async, static_cast<NewGameFromSave>(&GameWindow::LoadSaveFile), this, save_slots[selected_fidx].Directory);
            StartNewGameLoadingScreen();
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

    this->CheckNewGameProgress();
    this->RenderNewGameLoadingScreen();

    if (NewGameResult.has_value()) {
        if (NewGameResult.value())
            ImGui::CloseCurrentPopup();
        else {
            ImGui::OpenPopup("Invalid File!##ErrorWindowLSF");
            ImGui::SetNextWindowSize(ImVec2(400.0f, 96.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }
        NewGameResult.reset();
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

void GameWindow::RenderNewGameLoadingScreen()
{
    if (StartLoadingScreen) {
        NewGameLoading.StartNewGameLoadingScreen(ImVec2(120.0f, 110.0f));
        StartLoadingScreen = false;
    }
    NewGameLoading.RenderLoading(ShowLoadingScreen, 40.0f, 0.0f, 5.0f);
}

//---------------------------------------------------------
// OPTIONS
//---------------------------------------------------------

void GameWindow::PencilmarkOptions()
{
    if (ImGui::Button(ShowPencilmarks ? "Hide Pencilmarks" : "Show Pencilmarks", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        ShowPencilmarks = !ShowPencilmarks;
        if (!ShowSolution && ShowPencilmarks) {
            for (auto& row_tile : SudokuGameTiles)
                for (auto& tile : row_tile)
                    tile.UpdateTileNumber(TileState_Pencilmark);
        }
    }

    ImGui::BeginDisabled(!ShowPencilmarks);
    if (ImGui::Button("Reset Pencilmarks", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 0.0f))) {
        ImGui::OpenPopup("Are You Sure##ResetPencilmark");
        ImGui::SetNextWindowSize(ImVec2(360.0f, 97.0f), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }
    
    if (ImGui::BeginPopupModal("Are You Sure##ResetPencilmark", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        CenterText("This action will reset and replace current pencilmarks.");
        CenterText("This action cannot be undone. Proceed?");
    
        constexpr ImVec2 button_size(50.0f, 0.0f);
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - button_size.x * 2.0f) * 0.50f);
        if (ImGui::Button("Yes##pmcconfirm", button_size)) {
            SudokuContext.ResetAllPencilmarks();
            SudokuContext.ResetTurnLogs();
            ImGui::CloseCurrentPopup();
        }
    
        ImGui::SameLine();
        if (ImGui::Button("No##pmconfirm", button_size))
            ImGui::CloseCurrentPopup();
    
        ImGui::EndPopup();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear All Pencilmarks", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
        ImGui::OpenPopup("Are You Sure##ClearPencilmark");
        ImGui::SetNextWindowSize(ImVec2(360.0f, 97.0f), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }
    
    if (ImGui::BeginPopupModal("Are You Sure##ClearPencilmark", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        CenterText("This action will remove current pencilmarks.");
        CenterText("This action cannot be undone. Proceed?");
    
        constexpr ImVec2 button_size(50.0f, 0.0f);
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - button_size.x * 2.0f) * 0.50f);
        if (ImGui::Button("Yes##pmcconfirm", button_size)) {
            SudokuContext.ClearAllPencilmarks();
            SudokuContext.ResetTurnLogs();
            ImGui::CloseCurrentPopup();
        }
    
        ImGui::SameLine();
        if (ImGui::Button("No##pmconfirm", button_size))
            ImGui::CloseCurrentPopup();
    
        ImGui::EndPopup();
    }
    ImGui::EndDisabled();
}

void GameWindow::UndoRedoOptions()
{
    ImGui::BeginDisabled(!SudokuContext.GetTurnLogs()->CanUndo());
    if (ImGui::Button("Undo", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 0.0f))) {
        if (const auto* undo_tile = SudokuContext.GetTurnLogs()->GetUndoTile()) {
            SudokuContext.UndoTurn();
            SudokuGameTiles[undo_tile->Row][undo_tile->Column].UpdateTileNumber(ShowPencilmarks ? TileState_Pencilmark : TileState_Normal);
            RecheckTiles();
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!SudokuContext.GetTurnLogs()->CanRedo());
    if (ImGui::Button("Redo", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        if (const auto* redo_tile = SudokuContext.GetTurnLogs()->GetRedoTile()) {
            SudokuContext.RedoTurn();
            SudokuGameTiles[redo_tile->Row][redo_tile->Column].UpdateTileNumber(ShowPencilmarks ? TileState_Pencilmark : TileState_Normal);
            RecheckTiles();
        }
    }
    ImGui::EndDisabled();
}

void GameWindow::TimeOptions()
{
    ImGui::BeginDisabled(!GameStart);
    if (ImGui::Button(GamePaused ? "Resume Game" : "Pause Game", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, 0))) {
        GamePaused = !GamePaused;
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart Game", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        ImGui::OpenPopup("Restart Game##Popup");
        ImGui::SetNextWindowSize(ImVec2(360.0f, 97.0f), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Restart Game##Popup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        CenterText("This action will reset every progress you've made!");
        CenterText("Are you sure?");

        constexpr ImVec2 button_size(100.0f, 0.0f);
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - (100.0f * 2.0f)) * 0.5f);
        if (ImGui::Button("Restart", button_size)) {
            GamePaused      = false;
            ShowError       = false;
            ShowSolution    = false;
            ShowPencilmarks = false;
            TimeElapsed.Clear();

            for (size_t row = 0; row < 9; ++row) {
                for (size_t col = 0; col < 9; ++col) {
                    if (!SudokuGameTiles[row][col].IsPuzzleTile())
                        continue;

                    SudokuContext.ResetTile(row, col);
                    SudokuGameTiles[row][col].UpdateTileNumber(TileState_Normal);
                }
            }

            SudokuContext.ResetTurnLogs();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", button_size))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
    ImGui::EndDisabled();
}

void GameWindow::ErrorAndSolutionOptions()
{
    ImGui::Checkbox("Show Errors", &ShowError);
    ImGui::SameLine();
    static const std::string showerror_help =
        "Shows all the current and upcoming errors. "
        "Errored tiles are red but will not be replaced by the correct number!";
    HelpToolTip(showerror_help);
    ImGui::SameLine();
    if (ImGui::Checkbox("Show Solution", &ShowSolution))
        this->SetShowSolution();
    ImGui::SameLine();
    static const std::string checksolution_help =
        "Shows the puzzle's solution. "
        "Errored tiles will be in red and will be replaced by the correct number!\n"
        "This is kind of cheating so... Do not use this often.";
    HelpToolTip(checksolution_help);
}

void GameWindow::SaveProgressOption()
{
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
}

void GameWindow::SavePuzzleOption()
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

void GameWindow::NewGameOption()
{
    constexpr std::array<const char*, 7> game_difficulties = { "Random", "Easy As Pie", "Normal And Average", "Insane Enjoyer", "Diabolical Chad", "Load A Sudoku Puzzle", "Load A Save File" };
    const char* current_difficulty = game_difficulties[GameDifficulty];

    ImGui::Text("Sudoku Difficulty: %s", game_difficulties[SudokuContext.GetBoardDifficulty()]);
    ImGui::PushItemWidth(190.0f);
    if (ImGui::BeginCombo("##Sudoku Difficulty", current_difficulty)) {
        for (int n = 0; n < game_difficulties.size(); n++) {
            const bool is_selected = GameDifficulty == n;
            if (ImGui::Selectable(game_difficulties[n], is_selected))
                GameDifficulty = n;
            if (ImGui::IsItemHovered()) {
                static std::string difficulty_tooltip;
                switch (n)
                {
                case 0: difficulty_tooltip = "Generates a sudoku board of random difficulty.";                   break;
                case 1: difficulty_tooltip = "Generates an easy puzzle. Best for beginners";                     break;
                case 2: difficulty_tooltip = "Generates a normal puzzle. Best for passing time.";                break;
                case 3: difficulty_tooltip = "Generates an insane puzzle. Best for passionate problem solvers."; break;
                case 4: difficulty_tooltip = "Generates a really sadistic puzzle. Best for the chad of chads.";  break;
                case 5: difficulty_tooltip = "Loads a sudoku puzzle from file.";                                 break;
                case 6: difficulty_tooltip = "Loads a saved puzzle progress.";                                   break;
                default: assert(false, "Invalid difficulty choice!");  break;
                }

                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(250.0f);
                ImGui::TextUnformatted(difficulty_tooltip.data());
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("New Game", ImVec2(80.0f, 0.0f))) {
        if (GameDifficulty == 5)
            OpenLoadSudokuWindow = true;
        else if (GameDifficulty == 6) {
            OpenLoadSaveFileWindow = true;
            LoadAFile = true;
        }
        else {
            using NewGameFromDifficulty = bool(GameWindow::*)(int);
            NewGameFuture = std::async(std::launch::async, static_cast<NewGameFromDifficulty>(&GameWindow::CreateNewGame), this, GameDifficulty);
            NewGameLoading.StartNewGameLoadingScreen(ImVec2(120.0f, 110.0f));
            StartLoadingScreen = true;
        }
    }

    if (GameDifficulty < 5) {
        this->CheckNewGameProgress();
        this->RenderNewGameLoadingScreen();

        if (NewGameResult.has_value())
            NewGameResult.reset();
    }
}

void GameWindow::GameOptions()
{
    if (!ImGui::BeginChild("GameOptionsWindow")) {
        ImGui::EndChild();
        return;
    }

    this->NewGameOption();

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 2.00f);
    if (ImGui::CollapsingHeader("Puzzle")) {
        ImGui::Indent();
        this->TimeOptions();
        ImGui::BeginDisabled(!GameStart || GamePaused);
        this->UndoRedoOptions();
        this->ErrorAndSolutionOptions();
        ImGui::EndDisabled();
        ImGui::Unindent();
    }
    if (ImGui::CollapsingHeader("Pencilmarks")) {
        ImGui::Indent();
        ImGui::BeginDisabled(!GameStart || GamePaused);
        this->PencilmarkOptions();
        ImGui::EndDisabled();
        ImGui::Unindent();
    }
    if (ImGui::CollapsingHeader("File")) {
        ImGui::Indent();
        ImGui::BeginDisabled(!GameStart);
        this->SaveProgressOption();
        ImGui::EndDisabled();
        this->SavePuzzleOption();
        ImGui::Unindent();
    }
    ImGui::PopStyleVar();

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
    if (ShowPencilmarks) {
        for (auto& row_tile : SudokuGameTiles)
            for (auto& tile : row_tile)
                tile.UpdateTileNumber(TileState_Normal);
    }
    this->StopOngoingGame();
    if (!SudokuContext.CreateSudoku(difficulty))
        return false;

    GameStart = true;
    SudokuFileSaved = false;
    CurrentlyOpenFile = "None";
    this->SetShowSolution();
    this->SetSudokuTilesForNewGame();

    return true;
}

bool GameWindow::CreateNewGame(const std::string& filepath)
{
    std::lock_guard func_guard(NewGameMutex);
    const auto& input_sudoku_board = sdq::utils::OpenSudokuFile(filepath.c_str());
    if (!input_sudoku_board.has_value())
        return false;

    if (ShowPencilmarks) {
        for (auto& row_tile : SudokuGameTiles)
            for (auto& tile : row_tile)
                tile.UpdateTileNumber(TileState_Normal);
    }
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
    GameStart       = false;
    GamePaused      = false;
    ShowSolution    = false;
    ShowError       = false;
    ShowPencilmarks = false;
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
            SudokuGameTiles[row][col].UpdateTileNumber(ShowSolution ? TileState_Solution : ShowPencilmarks ? TileState_Pencilmark : TileState_Normal);
}

void GameWindow::RecheckTiles()
{
    for (size_t row = 0; row < 9; row++)
        for (size_t col = 0; col < 9; ++col)
            SudokuGameTiles[row][col].RecheckError(SudokuContext, row, col);
}

bool GameWindow::SaveProgress(const std::string& filepath)
{
    return SudokuContext.SaveCurrentProgress(filepath.data());
}

void GameWindow::Update()
{
    ImGuiIO io = ImGui::GetIO();

    if (GameStart && !GamePaused)
        TimeElapsed += io.DeltaTime;

    if (ShowSolution)
        ShowSolutionTotalTime += io.DeltaTime;

    if (CheckGameState) {
        if (SudokuContext.CheckPuzzleState()) {
            OpenGameEndWindow = true;
            StopOngoingGame();
        }
        else {
            RecheckTiles();
        }
        CheckGameState = false;
    }
}

bool GameWindow::LoadSaveFile(const std::string& filepath)
{
    std::lock_guard func_guard(NewGameMutex);
    if (ShowPencilmarks) {
        for (auto& row_tile : SudokuGameTiles)
            for (auto& tile : row_tile)
                tile.UpdateTileNumber(TileState_Normal);
    }
    if(!SudokuContext.LoadSudokuSave(filepath.data()))
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

void GameWindow::StartNewGameLoadingScreen()
{
    StartLoadingScreen = true;
    NewGameResult = std::nullopt;
}

void GameWindow::CheckNewGameProgress()
{
    if (NewGameFuture.valid()) {
        ShowLoadingScreen = NewGameFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
        if (!ShowLoadingScreen)
            NewGameResult = NewGameFuture.get();
    }
}



//-----------------------------------------------------------------------------------------------------------------------------------------------
// SudokuTile CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------

SudokuTile::SudokuTile() :
    Initialized(false),
    ErrorTile(false),
    PuzzleTile(true),
    ShowAsSolution(false),
    ShowAsPencilmark(false),
    Pencilmark(nullptr),
    SolutionNumber(0),
    TileNumber(0), 
    TileID(0)
{}

SudokuTile::SudokuTile(const sdq::BoardTile& puzzle_tile, const sdq::BoardTile& solution_tile)
{
    InitializeTile(puzzle_tile, solution_tile);
}

bool SudokuTile::InitializeTile(const sdq::BoardTile& puzzle_tile, const sdq::BoardTile& solution_tile)
{
    if (Initialized)
        return true;

    TileID         = (puzzle_tile.Row * 9) + puzzle_tile.Column;
    TileNumber     = &puzzle_tile.TileNumber;
    SolutionNumber = &solution_tile.TileNumber;
    Pencilmark     = &puzzle_tile.Pencilmarks;
    ErrorTile      = false;
    PuzzleTile     = false;
    // A series of early out statements in case the sprintf_s fails
    bool success = *TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", *TileNumber, TileID) > 0 : sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) > 0;
    if (!success)
        return false;
    if (sprintf_s(ContextPopUpLabel, CharBufferSize, "##CPU%d", TileID) < 0)
        return false;
    if (sprintf_s(InputIntLabel, CharBufferSize, "##II%d", TileID) < 0)
        return false;

    Initialized = true;
    return true;
}

void SudokuTile::UpdateTileNumber(TileState tile_state)
{
    if (!PuzzleTile)
        return;

    if (tile_state == TileState_Pencilmark && *TileNumber != 0)
        tile_state = TileState_Normal;

    switch (tile_state)
    {
    case TileState_Normal:
        ShowAsSolution   = false;
        ShowAsPencilmark = false;
        InputTileNumber  = *TileNumber;
        *TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", *TileNumber, TileID) : sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID);
        break;
    case TileState_Solution:
        ShowAsSolution = true;
        sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", *SolutionNumber, TileID);
        break;
    case TileState_Pencilmark:    
        ShowAsPencilmark = true;
        ShowAsSolution = false;
        sprintf_s(ButtonLabel, "##B%d", TileID);
        break;
    default:
        assert(false, "Invalid TileState used!");
    }
}

void SudokuTile::SetTilePuzzleNumber(uint16_t t_number, uint16_t solution_number, bool is_puzzle)
{
    ErrorTile       = false;
    PuzzleTile      = is_puzzle;
    InputTileNumber = *TileNumber;
    *TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", *TileNumber, TileID) : sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID);
}

bool SudokuTile::RenderButton(sdq::Instance& sudoku_context, uint16_t row, uint16_t col, bool error_override)
{
    bool value_changed = false;
    constexpr ImU32 error_button_col        = 2600468659;
    constexpr ImU32 error_buttonhovered_col = 2721396170;
    constexpr ImU32 error_buttonactive_col  = 2267161319;
    const bool error_tile                   = (ErrorTile && error_override) || (ShowAsSolution && *TileNumber != *SolutionNumber);
    const bool show_as_pencilmark           = *TileNumber == 0 && !ShowAsSolution && PuzzleTile && ShowAsPencilmark;
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, ShowAsSolution ? 0.90f : 0.80f);
    ImGui::PushStyleColor(ImGuiCol_Button       , error_tile ? error_button_col        : ImGui::GetColorU32(ImGuiCol_Button));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, error_tile ? error_buttonhovered_col : ImGui::GetColorU32(ImGuiCol_ButtonHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive , error_tile ? error_buttonactive_col  : ImGui::GetColorU32(ImGuiCol_ButtonActive));
    ImGui::BeginDisabled(!PuzzleTile || ShowAsSolution);

    static int pencilmark_num = 0;
    static int pressed_pm_num = 0;
    ImGui::PushFont(show_as_pencilmark ? ImGui::GetIO().Fonts->Fonts[3] : ImGui::GetIO().Fonts->Fonts[2]);
    if (show_as_pencilmark ? pressed_pm_num = PencilmarkButton(ButtonLabel, ButtonSize, *Pencilmark) : ImGui::Button(ButtonLabel, ButtonSize)) {
        pencilmark_num = pressed_pm_num;
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImGui::OpenPopup(ContextPopUpLabel);
        ImGui::SetNextWindowPos(ImVec2(mouse_pos.x + 15.0f, mouse_pos.y), ImGuiCond_Always);
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    if (ImGui::BeginPopup(ContextPopUpLabel)) {
        if (!ShowAsPencilmark) {
            ImGui::PushItemWidth(25.0f);
            ImGui::SetKeyboardFocusHere();
            ImGui::InputInt(InputIntLabel, &InputTileNumber, 0, 0);
            if (ImGui::IsKeyPressed(526, false)) {
                ImGui::CloseCurrentPopup();
            }
            else if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsKeyPressed(525, false)) {
                if (InputTileNumber < 0) 
                    InputTileNumber = 0;
                else if (InputTileNumber > 9) 
                    InputTileNumber = 9;

                if (sudoku_context.SetTile(row, col, InputTileNumber))
                    value_changed = true;

                ImGui::CloseCurrentPopup();
            }
        }
        else {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.50f, 2.50f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.50f, 2.50f));
            static char pencilmark_text[32];
            sprintf_s(pencilmark_text, "Tile %c%d PencilMark #%d", 'A' + row, col + 1, pencilmark_num);
            ImGui::TextUnformatted(pencilmark_text);
            const bool pencilmark_removed = (*this->Pencilmark)[pencilmark_num - 1];
            ImGui::BeginDisabled(pencilmark_removed);
            if (ImGui::Button("Remove Pencilmark", ImVec2(125.0f, 0))) {
                sudoku_context.RemovePencilmark(row, col, pencilmark_num);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();
            ImGui::BeginDisabled(!pencilmark_removed);
            if (ImGui::Button("Add Pencilmark", ImVec2(125.0f, 0))) {
                sudoku_context.AddPencilmark(row, col, pencilmark_num);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();
            ImGui::BeginDisabled(pencilmark_removed);
            if (ImGui::Button("Finalize Pencilmark", ImVec2(125.0f, 0))) {
                if (sudoku_context.SetTile(row, col, pencilmark_num))
                    value_changed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();
            ImGui::PopStyleVar(2);
            ImGui::PopFont();
        }
        ImGui::EndPopup();
    }
    ImGui::EndDisabled();
    ImGui::PopStyleVar();

    return value_changed;
}

bool SudokuTile::IsPuzzleTile()
{
    return PuzzleTile;
}

bool SudokuTile::IsTileFilled()
{
    return TileNumber != 0;
}

void SudokuTile::RecheckError(sdq::Instance& sudoku_context, uint16_t row, uint16_t col)
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

template<size_t S>
static void SimpleComboWrapper(const char* label, const std::array<const char*, S>& choices, int& current_choice)
{
    if (ImGui::BeginCombo(label, choices[current_choice])) {
        for (int n = 0; n < choices.size(); n++) {
            const bool is_selected = (current_choice == n);
            if (ImGui::Selectable(choices[n], is_selected))
                current_choice = n;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

static int PencilmarkButton(const char* label, const ImVec2& sz, const std::bitset<9>& pencilmark)
{
    using namespace ImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImGuiButtonFlags flags = ImGuiButtonFlags_None;

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(sz, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    //RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_Button), true, style.FrameRounding);

    int return_pencilmark = 0;
    const ImVec2 mini_square_size = sz / 3.0f;
    for (size_t idx = 0; idx < 9; ++idx) {
        const ImVec2 min_square_pos = ImVec2(pos.x + (mini_square_size.x * (idx % 3)), pos.y + (mini_square_size.y * (idx / 3)));
        const ImVec2 max_square_pos = ImVec2(min_square_pos.x + mini_square_size.x, min_square_pos.y + mini_square_size.y);

        const ImVec2 current_mouse_pos = GetMousePos();
        const bool is_hovered = current_mouse_pos.x >= min_square_pos.x && current_mouse_pos.x < max_square_pos.x && 
                                current_mouse_pos.y >= min_square_pos.y && current_mouse_pos.y < max_square_pos.y;
        char pencilmark_label[36];
        pencilmark[idx] ? sprintf_s(pencilmark_label, " ##%s", label) : sprintf_s(pencilmark_label, "%zu##%s", idx + 1, label);
        const ImVec2 text_size = CalcTextSize(pencilmark_label, nullptr, true);
        RenderFrame(min_square_pos, max_square_pos, is_hovered ? col : GetColorU32(ImGuiCol_Button), false, style.FrameRounding);
        RenderTextClipped(min_square_pos + style.FramePadding, max_square_pos - style.FramePadding, pencilmark_label, nullptr, &text_size, ImVec2(0.65f, 0.50f));
        if (pressed && is_hovered)
            return_pencilmark = idx + 1;
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return return_pencilmark;
}





