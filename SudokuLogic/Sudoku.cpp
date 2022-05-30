#include "Sudoku.h"
#include <unordered_set>

// Functions for querying the sudoku board rows/columns/cells
namespace sdq
{

constexpr uint16_t GetNextRow(uint16_t row, uint16_t col) noexcept
{
    return row + (col + 1) / 9;
}

constexpr uint16_t GetNextCol(uint16_t col) noexcept
{
    return (col + 1) % 9;
}

constexpr std::pair<uint16_t, uint16_t> NextEmptyPosition(const SudokuBoard& board, uint16_t row, uint16_t col) noexcept
{
    while (row != 9) {
        if (board[row][col] == '0') {
            return { row, col };
        }
        row = sdq::GetNextRow(row, col);
        col = sdq::GetNextCol(col);
    }

    return { 9, 0 };
}

void ClearSudokuBoard(SudokuBoard& board)
{
    std::vector<char> empty_vec(9);
    std::fill(empty_vec.begin(), empty_vec.end(), '0');
    std::fill(board.begin(), board.end(), empty_vec);
}

}

SudokuBoardLogic::SudokuBoardLogic() :
    RowNumbers({0, 0, 0, 0, 0, 0, 0, 0, 0}),
    ColNumbers({0, 0, 0, 0, 0, 0, 0, 0, 0}),
    CellNumbers({0, 0, 0, 0, 0, 0, 0, 0, 0})
{}

void SudokuBoardLogic::ResetAll()
{
    RowNumbers = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ColNumbers = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    CellNumbers = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
}

constexpr uint16_t SudokuBoardLogic::GetCellBlock(uint16_t row, uint16_t col) const noexcept
{
    return (row / 3) * 3 + col / 3;
}

void SudokuBoardLogic::SetCellNumber(uint16_t row, uint16_t col, uint16_t number) noexcept
{
    RowNumbers[row].set(number);
    ColNumbers[col].set(number);
    const auto& cellblock = this->GetCellBlock(row, col);
    CellNumbers[cellblock].set(number);
}

auto SudokuBoardLogic::GetRowNumber(uint16_t row) const  
{
    return RowNumbers[row];
}

auto SudokuBoardLogic::GetColNumber(uint16_t col) const  
{
    return ColNumbers[col];
}

auto SudokuBoardLogic::GetCellNumber(uint16_t cell) const
{
    return CellNumbers[cell];
}

auto SudokuBoardLogic::GetTileAvailability(uint16_t row, uint16_t col) const
{
    const auto& cell = this->GetCellBlock(row, col);
    return RowNumbers[row] | ColNumbers[col] | CellNumbers[cell];
}

void SudokuBoardLogic::ResetCellNumber(uint16_t row, uint16_t col, uint16_t number) noexcept
{
    RowNumbers[row].reset(number);
    ColNumbers[col].reset(number);
    const auto& cell = this->GetCellBlock(row, col);
    CellNumbers[cell].reset(number);
}



//--------------------------------------------------------------------------------------------------------------------------------
// Sudoku CLASS
//--------------------------------------------------------------------------------------------------------------------------------

Sudoku::Sudoku() :
    SolutionBoard(9),
    GameDifficulty(2),
    RandomDifficulty(0),
    MaximumIterations((size_t) - 1),
    MinimumIterations(0)
{}

bool Sudoku::InitializeGame(SudokuBoard& board)
{
    SolutionBoard = board;

    if (!this->IsValidBoard()) {
        return false;
    }

    PuzzleBoard = SolutionBoard;
    PuzzleLogic = SolutionLogic;

    std::cout << "INVALID\n";
    return false;
}

bool Sudoku::InitializeGame(SudokuDifficulty game_difficulty)
{
    this->InitializeGameParameters(game_difficulty);  // Initialize important game parameters for creating a sudoku puzzle
    this->InitializedGameSeed();                      // Initialize game seed
    return this->CreateSudoku();                      // Create sudoku puzzle
}

SudokuDifficulty Sudoku::CheckPuzzleDifficulty()
{
    auto tempboard = PuzzleBoard;
    auto templogic = PuzzleLogic;
    size_t iterations = 0;
    SolveSudoku(tempboard, templogic, 0, 0, iterations);

    if (GameDifficulty == SudokuDifficulty_Random) {
        if (iterations >= 1 && iterations <= 1000) {
            RandomDifficulty = SudokuDifficulty_Easy;
        }
        else if (iterations >= 1001 && iterations <= 10000) {
            RandomDifficulty = SudokuDifficulty_Normal;
        }
        else if (iterations >= 10001 && iterations <= 20000) {
            RandomDifficulty = SudokuDifficulty_Hard;
        }
        else {
            RandomDifficulty = SudokuDifficulty_ChadBrain;
        }
        return SudokuDifficulty_Random;
    }

    if (iterations < MinimumIterations || iterations > MaximumIterations) {
        return SudokuDifficulty_Not;
    }

    return GameDifficulty;
}

void Sudoku::InitializeGameParameters(SudokuDifficulty game_difficulty)
{
    this->GameDifficulty = game_difficulty;
    switch (game_difficulty)
    {
    case SudokuDifficulty_Random: {
        std::uniform_int_distribution<> max_remove_range(46, 64);
        MaxRemovedTiles   = max_remove_range(GameRNG);
        MinimumIterations = 0;
        MaximumIterations = (size_t) - 1;
        break;
    }
    case SudokuDifficulty_Easy:
        MaxRemovedTiles   = 46;
        MinimumIterations = 1;
        MaximumIterations = 1000;
        break;
    case SudokuDifficulty_Normal:
        MaxRemovedTiles   = 51;
        MinimumIterations = 1001;
        MaximumIterations = 10000;
        break;
    case SudokuDifficulty_Hard:
        MaxRemovedTiles   = 58;
        MinimumIterations = 10001;
        MaximumIterations = 20000;
        break;
    case SudokuDifficulty_ChadBrain:
        MaxRemovedTiles   = 64;
        MinimumIterations = 20001;
        MaximumIterations = (size_t) - 1;
        break;
    default:
        MaxRemovedTiles   = 51;
        MinimumIterations = 1001;
        MaximumIterations = 2500;
        break;
    }
}

void Sudoku::InitializedGameSeed()
{
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    GameRNG.seed(seed);
}

void Sudoku::ClearSudokuBoard()
{
    SolutionLogic.ResetAll();
    std::vector<char> empty_vec(9);
    std::fill(empty_vec.begin(), empty_vec.end(), '0');
    std::fill(SolutionBoard.begin(), SolutionBoard.end(), empty_vec);
}

bool Sudoku::IsValidBoard()
{
    SolutionLogic.ResetAll();

    size_t filled_tile = 0;
    for (uint16_t row = 0; row < 9; ++row) {
        for (uint16_t col = 0; col < 9; ++col) {
            char digit;
            if ((digit = SolutionBoard[row][col]) == '0') {
                continue;
            }

            uint16_t digit_num = digit - '1';
            SolutionLogic.SetCellNumber(row, col, digit_num);
            ++filled_tile;
        }
    }

    size_t actual_filled_tile = 0;
    //for (uint16_t row = 0; row < 9; ++row) {
    //    uint16_t col = row;
    //}
    for (uint16_t cell = 0; cell < 9; ++cell) {
        const auto& c = SolutionLogic.GetCellNumber(cell);
        for (size_t idx = 0; idx < 9; ++idx) {
            actual_filled_tile += c[idx];
        }
    }

    return filled_tile == actual_filled_tile;
}

bool Sudoku::CreateSudoku()
{
    // Fill the grid with 27 numbers (three cells worth of numbers)
    FillInitialBoard();

    // Solve the initial grid. If false, the initial board is either unsolvable or with multiple solutions
    if (!SolveSudoku(SolutionBoard, SolutionLogic, 0, 0)) {
        return false;
    }

    // Start making the puzzle by removing certain numbers
    GeneratePuzzle();

    return true;
}

void Sudoku::FillInitialBoard()
{
    ClearSudokuBoard();
    static std::array<char, 9> board_numbers  = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

    auto fill_diagonal_cells = [&](int start_row, int end_row, int start_col, int end_col) {
        std::shuffle(board_numbers.begin(), board_numbers.end(), GameRNG);
        int num_idx = 0;
        for (int row = start_row; row < end_row; ++row) {
            for (int col = start_col; col < end_col; ++col) {
                SolutionBoard[row][col] = board_numbers[num_idx];
                SolutionLogic.SetCellNumber(row, col, board_numbers[num_idx] - '1');
                num_idx++;
            }
        }
    };

    fill_diagonal_cells(0, 3, 0, 3); // We will fill the three      x o o
    fill_diagonal_cells(3, 6, 3, 6); // non-connecting diagonal     o x o
    fill_diagonal_cells(6, 9, 6, 9); // cells of the sudoku board   o o x
}

void Sudoku::GeneratePuzzle()
{
    PuzzleBoard = SolutionBoard;
    PuzzleLogic = SolutionLogic;

    constexpr size_t max_number_of_tiles = 81;
    static std::array<std::pair<uint16_t, uint16_t>, max_number_of_tiles> tiles_to_be_removed = { { {0, 0},{0, 1},{0, 2},{0, 3},{0, 4},{0, 5},{0, 6},{0, 7},{0, 8},{1, 0},{1, 1},{1, 2},{1, 3},{1, 4},{1, 5},{1, 6},{1, 7},{1, 8},{2, 0},{2, 1},{2, 2},{2, 3},{2, 4},{2, 5},{2, 6},{2, 7},{2, 8},{3, 0},{3, 1},{3, 2},{3, 3},{3, 4},{3, 5},{3, 6},{3, 7},{3, 8},{4, 0},{4, 1},{4, 2},{4, 3},{4, 4},{4, 5},{4, 6},{4, 7},{4, 8},{5, 0},{5, 1},{5, 2},{5, 3},{5, 4},{5, 5},{5, 6},{5, 7},{5, 8},{6, 0},{6, 1},{6, 2},{6, 3},{6, 4},{6, 5},{6, 6},{6, 7},{6, 8},{7, 0},{7, 1},{7, 2},{7, 3},{7, 4},{7, 5},{7, 6},{7, 7},{7, 8},{8, 0},{8, 1},{8, 2},{8, 3},{8, 4},{8, 5},{8, 6},{8, 7},{8, 8} } };
    // Shuffle the array so that it would not just remove tiles from the top left to bottom right
    // because there is also a stop flag when a certain number of removed tiles is reached
    std::shuffle(tiles_to_be_removed.begin(), tiles_to_be_removed.end(), GameRNG);

    int removed_tiles = 0;
    for (int i = 0; i < max_number_of_tiles && removed_tiles < MaxRemovedTiles; ++i) {
        auto row = tiles_to_be_removed[i].first;
        auto col = tiles_to_be_removed[i].second;
        auto tile_num = PuzzleBoard[row][col];
        if (tile_num != '0') {
            this->PuzzleLogic.ResetCellNumber(row, col, tile_num - '1');
            this->PuzzleBoard[row][col] = '0';
            if (this->CountSolutions(PuzzleBoard, PuzzleLogic, 0, 0) > 1) {
                PuzzleBoard[row][col] = tile_num;
                PuzzleLogic.SetCellNumber(row, col, tile_num - '1');
                continue;
            }
            removed_tiles++;
        }
    }
    
    if (this->CheckPuzzleDifficulty() == SudokuDifficulty_Not) {
        return this->GeneratePuzzle();
    }
}

constexpr bool Sudoku::SolveSudoku(SudokuBoard& board, SudokuBoardLogic& board_logic, uint16_t const row_start, uint16_t const col_start, size_t& iterations) noexcept
{
    ++iterations;
    const auto& [row, col] = sdq::NextEmptyPosition(board, row_start, col_start);

    // end of board 
    if (row == 9) {
        return true;
    }

    std::bitset<9> const contains = board_logic.GetTileAvailability(row, col);
    if (contains.all()) {
        return false;
    }

    for (uint16_t digit_idx = 0; digit_idx < 9; ++digit_idx) {
        if (contains[digit_idx]) {
            continue;
        }

        board[row][col] = static_cast<char>(digit_idx + '1');
        board_logic.SetCellNumber(row, col, digit_idx);
        if (SolveSudoku(board, board_logic, row, col, iterations)) {
            return true;
        }
        board_logic.ResetCellNumber(row, col, digit_idx);
    }

    board[row][col] = '0';
    return false;
}

constexpr bool Sudoku::SolveSudoku(SudokuBoard& board, SudokuBoardLogic& board_logic, uint16_t const row_start, uint16_t const col_start) noexcept
{
    const auto& [row, col] = sdq::NextEmptyPosition(board, row_start, col_start);

    // end of board 
    if (row == 9) {
        return true;
    }

    std::bitset<9> const contains = board_logic.GetTileAvailability(row, col);
    if (contains.all()) {
        return false;
    }

    for (uint16_t digit_idx = 0; digit_idx < 9; ++digit_idx) {
        if (contains[digit_idx]) {
            continue;
        }

        board[row][col] = static_cast<char>(digit_idx + '1');
        board_logic.SetCellNumber(row, col, digit_idx);
        if (SolveSudoku(board, board_logic, row, col)) {
            return true;
        }
        board_logic.ResetCellNumber(row, col, digit_idx);
    }

    board[row][col] = '0';
    return false;
}

constexpr int Sudoku::CountSolutions(SudokuBoard& board, SudokuBoardLogic& board_logic, uint16_t const row_start, uint16_t const col_start)
{
    int number_of_solutions = 0;

    const auto& [row, col] = sdq::NextEmptyPosition(board, row_start, col_start);

    // end of board 
    if (row == 9) {
        ++number_of_solutions;
        return number_of_solutions;
    }

    std::bitset<9> const contains = board_logic.GetTileAvailability(row, col);

    for (uint16_t digit_idx = 0; digit_idx < 9 && number_of_solutions < 2; ++digit_idx) {
        if (contains[digit_idx]) {
            continue;
        }

        board[row][col] = static_cast<char>(digit_idx + '1');
        board_logic.SetCellNumber(row, col, digit_idx);
        number_of_solutions += this->CountSolutions(board, board_logic, row, col);
        board_logic.ResetCellNumber(row, col, digit_idx);
    }

    board[row][col] = '0';
    return number_of_solutions;
}

//----------------------------------------------------------------------
// Sudoku GETTERS
//----------------------------------------------------------------------

SudokuBoard& Sudoku::GetPuzzleBoard()
{
    return PuzzleBoard;
}

SudokuDifficulty Sudoku::GetGameDifficulty() const
{
    return GameDifficulty == SudokuDifficulty_Random ? RandomDifficulty : GameDifficulty;
}

SudokuBoard& Sudoku::GetSolutionBoard()
{
    return SolutionBoard;
}

//----------------------------------------------------------------------
// Sudoku SETTERS
//----------------------------------------------------------------------

bool Sudoku::SetTile(uint16_t row, uint16_t col, uint16_t number)
{
    // In case the input 
    auto tile_prev_number = PuzzleBoard[row][col];
    if (number == (tile_prev_number - '0')) {
        return true;
    }

    // Checks if the input number is valid. If not, still register but return false;
    if (number != 0) {
        const auto& is_used = PuzzleLogic.GetTileAvailability(row, col);
        if (is_used[number - 1]) {
            PuzzleBoard[row][col] = static_cast<char>(number + '0');
            return false;
        }
    }

    PuzzleBoard[row][col] = static_cast<char>(number + '0');
    number == 0 ? PuzzleLogic.ResetCellNumber(row, col, tile_prev_number - '0' - 1) : PuzzleLogic.SetCellNumber(row, col, number - 1);
    return true;
}

void Sudoku::SetGameDifficulty(SudokuDifficulty game_difficulty)
{
    GameDifficulty = game_difficulty;
}

bool Sudoku::CheckGameState() const
{
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            if (PuzzleBoard[row][col] != SolutionBoard[row][col]) {
                return false;
            }
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------------------------------------------------
// DEBUG FUNCTIONS
//--------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
void Sudoku::FillPuzzleBoard()
{
    PuzzleBoard = SolutionBoard;
    PuzzleLogic = SolutionLogic;
}
#endif



























