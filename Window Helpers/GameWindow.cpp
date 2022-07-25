#include "GameWindow.h"
#include "IconsFontAwesome5.h"
#include "imgui_internal.h"

static void HelpToolTip(std::string_view str);

GameWindow::GameWindow() :
    GameStart(false),
    GameEnd(false),
    WindowClose(false),
    Initialized(false),
    ShouldShowSolution(false),
    ShouldAlwaysShowError(false),
    GameDifficulty(SudokuDifficulty_Normal)
{}

bool GameWindow::OneTimeInitialize()
{
    size_t tile_id = 0;
    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            if (!SudokuGameTiles[row][col].InitializeTile(tile_id)) {
                Initialized = false;
                return false;
            }
            ++tile_id;
        }
    }

    Initialized = true;
    return true;
}

void GameWindow::RenderWindow()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 10.0f);
    this->MainMenuBar();

    if (!Initialized) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    if (!ImGui::Begin("Main Body", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        ImGui::End();
        return;
    }

    this->GameEndWindow();

    this->GameMainWindow();

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

bool GameWindow::IsWindowClosed()
{
    return WindowClose;
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

#ifdef _DEBUG
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %0.2f", io.Framerate);
#endif // _DEBUG

    ImGui::EndMainMenuBar();
}

bool GameWindow::InitializeGame()
{
    if (!SudokuContext.InitializeGame(GameDifficulty)) {
        return false;
    }

    GameStart             = true;
    GameEnd               = false;
    ShouldShowSolution    = false;
    ShouldAlwaysShowError = false;
    TimeElapsed.Clear();
    ShowSolutionTotalTime.Clear();
    this->SetShowSolution();
    this->SetSudokuTileNumbers();

    return true;
}

void GameWindow::GameOptions()
{
    if (!ImGui::BeginChild("GameOptionsWindow")) {
        ImGui::EndChild();
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
#ifdef _DEBUG
    constexpr std::array<const char*, 6> game_difficulties = { "Random", "Easy As Pie", "Average Normal Player", "Tomfoolery Hard", "Chad Insane Sudoku Enjoyer", "Custom Board" };
#else
    constexpr std::array<const char*, 5> game_difficulties = { "Random", "Easy As Pie", "Average Normal Player", "Tomfoolery Hard", "Chad Insane Sudoku Enjoyer" };
#endif // _DEBUG
    const char* current_difficulty = game_difficulties[GameDifficulty];

    ImGui::Text("Sudoku Difficulty: %s", game_difficulties[SudokuContext.GetGameDifficulty()]);
    ImGui::PushItemWidth(240.0f);
    if (ImGui::BeginCombo("##Sudoku Difficulty", current_difficulty)) {
        for (int n = 0; n < game_difficulties.size(); n++) {
            const bool is_selected = (GameDifficulty == n);
            if (ImGui::Selectable(game_difficulties[n], is_selected)) {
                GameDifficulty = n;
                SudokuContext.SetGameDifficulty(GameDifficulty);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    static const std::string difficulty_help =
        "Random - Generates a puzzle of random difficulty puzzle.\n\n"
        "Easy As Pie - Generates an easy puzzle. Best for beginners.\n\n"
        "Average Normal Player - Generates a normal puzzle. Best for passing time.\n\n"
        "Tomfoolery Hard - Generates a hard puzzle. Best for passionate problem solvers.\n\n"
        "Chad Insane Sudoku Enjoyer - Generates a really sadistic puzzle. Best for the chads of the chads.";
        //"Custom Board - Create your own sudoku puzzle!";
    ImGui::SameLine();
    HelpToolTip(difficulty_help);

#ifdef _DEBUG
    if (GameDifficulty == SudokuDifficulty_Custom) {
        static int number_of_remaining_tiles = 30;
        ImGui::Text("Number Of Remaining Tiles: %d", number_of_remaining_tiles);
        ImGui::SliderInt("RemainingTilesSlider", &number_of_remaining_tiles, 17, 50, "", ImGuiSliderFlags_AlwaysClamp);
    }
#endif // _DEBUG

    if (ImGui::Button("Game Start!!!")) {
        this->InitializeGame();
    }
    ImGui::SameLine();
    if (GameStart) { TimeElapsed += io.DeltaTime; }
    ImGui::Text("Time Elapsed: %zuhr : %zumin : %0.2fs", TimeElapsed.Hours, TimeElapsed.Minutes, TimeElapsed.Seconds);

    ImGui::BeginDisabled(!GameStart);
    if (ImGui::Checkbox("Show Solution", &ShouldShowSolution)) {
        this->SetShowSolution();
    }
    if (ShouldShowSolution) { ShowSolutionTotalTime += io.DeltaTime; }
    ImGui::SameLine();
    static const std::string checksolution_help =
        "Shows the puzzles solution. "
        "Errored tiles will be in red and will be replaced by the correct number!";
    HelpToolTip(checksolution_help);
    ImGui::SameLine();
    ImGui::Checkbox("Show Errors", &ShouldAlwaysShowError);
    ImGui::SameLine();
    static const std::string showerror_help =
        "Shows all the current and upcoming errors. "
        "Errored tiles are red but will not be replaced by the correct number!";
    HelpToolTip(showerror_help);

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

        constexpr std::array<const char*, 9> cell_choices = { "Cell 1: A1 - C3", "Cell 2: D1 - F3", "Cell 3: G1 - I3", "Cell 4: A4 - C6", "Cell 5: D4 - F6", "Cell 6: G4 - I6", "Cell 7: A7 - C9", "Cell 8: D7 - F9", "Cell 9: G7 - I9" };
        static int cell_clear_num = 0;
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Clear Cell: ");
        ImGui::SameLine(80.0f);
        ImGui::PushItemWidth(125.0f);
        combo_lambda("##ClearCellCombo", cell_choices, cell_clear_num);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##CellClear", ImGuiDir_Right)) {
            int actual_cell = cell_clear_num ;
            int min_row = (actual_cell / 3) * 3;
            int max_row = min_row + 3;
            int min_col = (actual_cell % 3) * 3;
            int max_col = min_col + 3;
            for (int row = min_row; row < max_row; ++row) {
                for (int col = min_col; col < max_col; ++col) {
                    SudokuGameTiles[row][col].ResetTile();
                }
            }
        }

        constexpr std::array<const char*, 9> row_choices = { "Row 1: A1 - I1", "Row 2: A2 - I2", "Row 3: A3 - I3", "Row 4: A4 - I4", "Row 5: A5 - I5", "Row 6: A6 - I6", "Row 7: A7 - I7", "Row 8: A8 - I8", "Row 9: A9 - I9" };
        static int row_clear_num = 0;
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Clear Row: ");
        ImGui::SameLine(80.0f);
        ImGui::PushItemWidth(125.0f);
        combo_lambda("##RowClearCombo", row_choices, row_clear_num);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##RowClear", ImGuiDir_Right)) {
            int actual_row = row_clear_num;
            for (int col = 0; col < 9; ++col) {
                SudokuGameTiles[actual_row][col].ResetTile();
            }
        }

        constexpr std::array<const char*, 9> column_choices = { "Column 1: A1 - A9", "Column 2: B1 - B9", "Column 3: C1 - C9", "Column 4: D1 - D9", "Column 5: E1 - E9", "Column 6: F1 - F9", "Column 7: G1 - G9", "Column 8: H1 - H9", "Column 9: I1 - I9" };
        static int column_clear_num = 0;
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Clear Column: ");
        ImGui::SameLine(80.0f);
        ImGui::PushItemWidth(125.0f);
        combo_lambda("##ColumnClearCombo", column_choices, column_clear_num);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##ColClear", ImGuiDir_Right)) {
            int actual_col = column_clear_num;
            for (int row = 0; row < 9; ++row) {
                SudokuGameTiles[row][actual_col].ResetTile();
            }
        }
    } ImGui::EndGroup();
    ImGui::PopStyleVar();

#ifdef _DEBUG
    if (ImGui::Button("Complete the puzzle!")) {
        SudokuContext.FillPuzzleBoard();
        if (SudokuContext.CheckGameState()) {
            GameStart = false;
            GameEnd = true;
        }
    }
#endif
    ImGui::EndDisabled();

    ImGui::EndChild();
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
                if (SudokuGameTiles[row][col].RenderButton(SudokuContext, row, col, ShouldAlwaysShowError || ShouldShowSolution)) {
                    if (SudokuContext.CheckGameState()) {
                        GameEnd               = true;
                        GameStart             = false;
                        ShouldShowSolution    = false;
                        ShouldAlwaysShowError = false;
                    }
                }
            }
        }
        ImGui::EndDisabled();

        ImGui::EndTable();
        ImGui::PopFont();
        ImGui::PopStyleVar(4);

        
        {   // Render a somewhat thick line for the table
            constexpr ImVec2 row_split1_pos    = ImVec2(9.50f   + 26.0f, 174.0f  + 25.0f);
            constexpr ImVec2 row_split1_sz     = ImVec2(457.0f  + 26.0f, 176.0f  + 25.0f);
            constexpr ImVec2 row_split2_pos    = ImVec2(9.50f   + 26.0f, 321.0f  + 25.0f);
            constexpr ImVec2 row_split2_sz     = ImVec2(457.0f  + 26.0f, 323.0f  + 25.0f);
            constexpr ImVec2 column_split1_pos = ImVec2(157.50f + 26.0f, 28.50f   + 25.0f);
            constexpr ImVec2 column_split1_sz  = ImVec2(159.50f + 26.0f, 470.50f + 25.0f);
            constexpr ImVec2 column_split2_pos = ImVec2(307.50f + 26.0f, 28.50f   + 25.0f);
            constexpr ImVec2 column_split2_sz  = ImVec2(309.50f + 26.0f, 470.50f + 25.0f);
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

void GameWindow::SetSudokuTileNumbers()
{
    const auto& puzzle_board = SudokuContext.GetPuzzleBoard();
    const auto& solution_board = SudokuContext.GetSolutionBoard();

    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            //const auto& tile_num = puzzle_board[row][col] - '0';
            //const auto& solution_num = solution_board[row][col] - '0';
            SudokuGameTiles[row][col].SetTilePuzzleNumber(puzzle_board[row][col] - '0', solution_board[row][col] - '0');
        }
    }
}

void FinishTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str);
void SolutionTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str);

void GameWindow::GameEndWindow()
{
    if (!GameEnd) {
        return;
    }

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
        FinishTimeNote(SudokuContext.GetGameDifficulty(), TimeElapsed, finish_time_note);
        SolutionTimeNote(SudokuContext.GetGameDifficulty(), ShowSolutionTotalTime, solution_time_note);
    }

    if (!ImGui::BeginPopupModal("Puzzle Finished!##GameEnd", nullptr)) {
        return;
    }

    auto center_text_lambda = [](const std::string_view str) {
        const auto& window_width = ImGui::GetWindowSize().x;
        const auto& text_width = ImGui::CalcTextSize(str.data()).x;

        ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
        ImGui::TextUnformatted(str.data());
    };

    center_text_lambda("Congratulations! You managed to finish the sudoku puzzle!");

    ImGui::Separator();
    center_text_lambda("Time taken to finish the puzzle:");
    char finish_time[64];
    sprintf_s(finish_time, 64, "%zu hours : %zu minutes : %0.2f seconds", TimeElapsed.Hours, TimeElapsed.Minutes, TimeElapsed.Seconds);
    center_text_lambda(finish_time);
    center_text_lambda(finish_time_note);

    ImGui::Separator();
    center_text_lambda("Total time taken to look at the solution:");
    char solution_look_time[64];
    sprintf_s(solution_look_time, 64, "%zu hours : %zu minutes : %0.2f seconds", ShowSolutionTotalTime.Hours, ShowSolutionTotalTime.Minutes, ShowSolutionTotalTime.Seconds);
    center_text_lambda(solution_look_time);
    center_text_lambda(solution_time_note);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 40.0f) * 0.50f);
    constexpr ImVec2 button_size = ImVec2(40.0f, 20.0f);
    if (ImGui::Button("Close", button_size)) {
        GameEnd         = false;
        GameStart       = false;
        init_modal_once = true;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void GameWindow::SetShowSolution()
{
    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            SudokuGameTiles[row][col].SetAsSolutionTile(ShouldShowSolution);
        }
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
    if (Initialized) {
        return false;
    }

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

void SudokuTile::SetTileNumber(uint16_t t_number)
{
    TileNumber = t_number;
    TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID) > 0 :
                      sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) > 0;
}

void SudokuTile::SetTilePuzzleNumber(uint16_t t_number, uint16_t solution_number)
{
    PuzzleTile = t_number != 0; 
    SolutionNumber = solution_number;
    this->SetTileNumber(t_number);
}

void SudokuTile::ResetTile()
{
    // Early out if its a puzzle number so it won't be changed
    if (PuzzleTile) { 
        return;
    }
    TileNumber = 0;
    ErrorTile  = false;
    sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID);
}

bool SudokuTile::RenderButton(Sudoku& sudoku_context, uint16_t row, uint16_t col, bool error_override)
{
    bool clicked = false;
    static const ImU32 error_button_col        = 2600468659;
    static const ImU32 error_buttonhovered_col = 2721396170;
    static const ImU32 error_buttonactive_col  = 2267161319;
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, ShowAsSolution ? 0.90f : 0.80f);
    ImGui::PushStyleColor(ImGuiCol_Button       , ErrorTile && error_override ? error_button_col        : ImGui::GetColorU32(ImGuiCol_Button));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ErrorTile && error_override ? error_buttonhovered_col : ImGui::GetColorU32(ImGuiCol_ButtonHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive , ErrorTile && error_override ? error_buttonactive_col  : ImGui::GetColorU32(ImGuiCol_ButtonActive));
    ImGui::BeginDisabled(PuzzleTile || ShowAsSolution);
    ImGui::Button(ButtonLabel, ButtonSize);
    ImGui::PopStyleColor(3);
    if (ImGui::BeginPopupContextItem(ContextPopUpLabel, 0)) {
        ImGui::PushItemWidth(25.0f);
        ImGui::SetKeyboardFocusHere();
        ImGui::InputInt(InputIntLabel, &TileNumber, 0, 0);
        if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsKeyPressed(525, false)) {
            if (TileNumber < 0) { TileNumber = 0; }
            if (TileNumber > 9) { TileNumber = 9; }
            TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID) < 0 : sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) < 0;
            ErrorTile = !sudoku_context.SetTile(row, col, TileNumber);
            clicked = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::EndDisabled();
    ImGui::PopStyleVar();

    return clicked && !ErrorTile;
}

void SudokuTile::SetAsSolutionTile(bool enable)
{
    if (!enable) {
        TileNumber != 0 ? sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", TileNumber, TileID) < 0 :
            sprintf_s(ButtonLabel, CharBufferSize, " ##B%d", TileID) < 0;
        ShowAsSolution = false;
        ErrorTile = TileNumber != 0 && TileNumber != SolutionNumber;
        return;
    }

    // Early out in case the tile is a puzzle tile since it's already the correct number
    if (PuzzleTile) {
        return;
    }
    sprintf_s(ButtonLabel, CharBufferSize, "%d##B%d", SolutionNumber, TileID);
    ErrorTile = TileNumber != 0 && TileNumber != SolutionNumber;
    ShowAsSolution = true;
}

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

void FinishTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str)
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
    case SudokuDifficulty_Hard:
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
    case SudokuDifficulty_ChadBrain:
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
        break;
    }
}

void SolutionTimeNote(SudokuDifficulty difficulty, const TimeObj& time_elapse, std::string& str)
{
    if (time_elapse.Minutes == 0 && time_elapse.Seconds == 0.0f && time_elapse.Hours == 0) {
        str = "Niceeeeeeeeeeeeee!";
    }
    else if (time_elapse.Seconds > 0.0f && time_elapse.Seconds < 30.0f && time_elapse.Hours == 0) {
        str = "Hmmmmmmmmmmmm. Forgivable";
    }
    else if (time_elapse.Seconds >= 30.0f && time_elapse.Seconds < 60.0f && time_elapse.Hours == 0) {
        str = "You had a bad time. It's barely forgivable though.";
    }
    else if (time_elapse.Minutes >= 1 && time_elapse.Minutes < 5 && time_elapse.Hours == 0) {
        str = "You must be having a reaaaaaaaaaaaaaaaaaally hard time solving the puzzle";
    }
    else {
        str = "Local kid looked at a sudoku solution for more than 5 minutes because they just can't handle it.";
    }
}




