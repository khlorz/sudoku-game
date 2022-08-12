#include "sdq.h"
#include <fstream>
#include <filesystem>

// Functions for querying the sudoku board rows/columns/cells
namespace sdq::helpers
{

constexpr int GetNextRow(int row, int col) noexcept
{
    return row + (col + 1) / 9;
}

constexpr int GetNextCol(int col) noexcept
{
    return (col + 1) % 9;
}

constexpr int GetCellBlock(int row, int col) noexcept
{
    return (row / 3) * 3 + col / 3;
}

constexpr std::tuple<int, int, int, int> GetMinMaxRowColumnFromCell(int const cell) noexcept
{
    int min_row, max_row;
    int min_col, max_col;

    switch (cell)
    {
    case 0:
        min_row = 0; max_row = 3;
        min_col = 0; max_col = 3;
        break;
    case 1:
        min_row = 0; max_row = 3;
        min_col = 3; max_col = 6;
        break;
    case 2:
        min_row = 0; max_row = 3;
        min_col = 6; max_col = 9;
        break;
    case 3:
        min_row = 3; max_row = 6;
        min_col = 0; max_col = 3;
        break;
    case 4:
        min_row = 3; max_row = 6;
        min_col = 3; max_col = 6;
        break;
    case 5:
        min_row = 3; max_row = 6;
        min_col = 6; max_col = 9;
        break;
    case 6:
        min_row = 6; max_row = 9;
        min_col = 0; max_col = 3;
        break;
    case 7:
        min_row = 6; max_row = 9;
        min_col = 3; max_col = 6;
        break;
    case 8:
        min_row = 6; max_row = 9;
        min_col = 6; max_col = 9;
        break;
    default:
        min_row = 0; max_row = 0;
        min_col = 0; max_col = 0;
        break;
    }

    return { min_row, max_row, min_col, max_col };
}

}

namespace sdq
{


//--------------------------------------------------------------------------------------------------------------------------------
// BoardOccurences CLASS
//--------------------------------------------------------------------------------------------------------------------------------

BoardOccurences::BoardOccurences() :
    RowOccurences({ 0, 0, 0, 0, 0, 0, 0, 0, 0 }),
    ColOccurences({ 0, 0, 0, 0, 0, 0, 0, 0, 0 }),
    CellOccurences({ 0, 0, 0, 0, 0, 0, 0, 0, 0 })
{}

constexpr bool BoardOccurences::IsEmpty() noexcept
{
    size_t total_count = 0;
    for (size_t idx = 0; idx < 9; ++idx) {
        total_count += RowOccurences[idx].count() + ColOccurences[idx].count() + CellOccurences[idx].count();
    }

    return total_count == 0;
}

void BoardOccurences::ResetAll()
{
    RowOccurences = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    ColOccurences = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    CellOccurences = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
}

void BoardOccurences::SetCellNumber(int row, int col, int number) noexcept
{
    RowOccurences[row].set(number);
    ColOccurences[col].set(number);
    const auto& cellblock = sdq::helpers::GetCellBlock(row, col);
    CellOccurences[cellblock].set(number);
}

std::bitset<9>& BoardOccurences::GetRowOccurences(int row) noexcept
{
    return RowOccurences[row];
}

std::bitset<9>& BoardOccurences::GetColumnOccurences(int col) noexcept
{
    return ColOccurences[col];
}

std::bitset<9>& BoardOccurences::GetCellOccurences(int cell) noexcept
{
    return CellOccurences[cell];
}

std::bitset<9> BoardOccurences::GetTileOccurences(int row, int col) const noexcept
{
    const auto& cell = sdq::helpers::GetCellBlock(row, col);
    return RowOccurences[row] | ColOccurences[col] | CellOccurences[cell];
}

void BoardOccurences::ResetCellNumber(int row, int col, int number) noexcept
{
    RowOccurences[row].reset(number);
    ColOccurences[col].reset(number);
    const auto& cell = sdq::helpers::GetCellBlock(row, col);
    CellOccurences[cell].reset(number);
}

std::bitset<9>& BoardOccurences::GetCellOccurences(int row, int col) noexcept
{
    const auto& cell = sdq::helpers::GetCellBlock(row, col);
    return CellOccurences[cell];
}


//--------------------------------------------------------------------------------------------------------------------------------
// BoardTile CLASS
//--------------------------------------------------------------------------------------------------------------------------------

BoardTile::BoardTile(int tile_number, int row, int col, BoardOccurences& board_occurences) noexcept
{
    this->Initialize(tile_number, row, col, board_occurences);
}

void BoardTile::Initialize(int tile_number, int row, int col, BoardOccurences& board_occurences) noexcept
{
    Row           = row;
    Column        = col;
    Cell          = sdq::helpers::GetCellBlock(row, col);
    TileNumber    = tile_number;
    RowOccurence  = &board_occurences.GetRowOccurences(row);
    ColOccurence  = &board_occurences.GetColumnOccurences(col);
    CellOccurence = &board_occurences.GetCellOccurences(row, col);
    Pencilmarks   = *RowOccurence | *ColOccurence | *CellOccurence;
}

void BoardTile::Clear() noexcept
{
    this->TileNumber  = 0;
    this->Pencilmarks = 0;
}

void BoardTile::SetTileNumber(int number) noexcept
{
    if (number == 0) {
        int prev_num = TileNumber - 1;
        RowOccurence->reset(prev_num);
        ColOccurence->reset(prev_num);
        CellOccurence->reset(prev_num);
    }
    else {
        if (TileNumber != 0) {
            int prev_bitnum = TileNumber - 1;
            RowOccurence->reset(prev_bitnum);
            ColOccurence->reset(prev_bitnum);
            CellOccurence->reset(prev_bitnum);
        }
        int bit_num = number - 1;
        RowOccurence->set(bit_num);
        ColOccurence->set(bit_num);
        CellOccurence->set(bit_num);
    }
    TileNumber = number;
}

void BoardTile::ResetTileNumber() noexcept
{
    SetTileNumber(0);
}

std::bitset<9> BoardTile::GetTileOccurences() noexcept
{
    return *RowOccurence | *ColOccurence | *CellOccurence;
}

void BoardTile::InitializeTileOccurences(BoardOccurences& board_occurences) noexcept
{
    RowOccurence = &board_occurences.GetRowOccurences(Row);
    ColOccurence = &board_occurences.GetColumnOccurences(Column);
    CellOccurence = &board_occurences.GetCellOccurences(Row, Column);
}

void BoardTile::RemovePencilmarks()
{
    Pencilmarks |= *RowOccurence | *ColOccurence | *CellOccurence;
}

void BoardTile::ReapplyPencilmarks()
{
    Pencilmarks &= *RowOccurence | *ColOccurence | *CellOccurence;
}

void BoardTile::ResetPencilmarks()
{
    Pencilmarks.reset();
    Pencilmarks |= *RowOccurence | *ColOccurence | *CellOccurence;
}

bool BoardTile::IsTileFilled() const noexcept
{
    return TileNumber != 0;
}

bool BoardTile::operator == (const BoardTile& other) const noexcept
{
    return this->Row == other.Row && this->Column == other.Column;
}



//--------------------------------------------------------------------------------------------------------------------------------
// SudokuBoard Structure
//--------------------------------------------------------------------------------------------------------------------------------

//----------------------------------
// SudokuBoard Constructors
//----------------------------------

GameBoard::GameBoard() : BoardInitialized(false)
{
    PuzzleTiles.reserve(64);
    BoardOccurences.ResetAll();
    for(int row = 0; row < 9; ++row)
        for(int col = 0; col < 9; ++col)
            BoardTiles[row][col].Initialize(0, row, col, BoardOccurences);
}

GameBoard::GameBoard(const GameBoard& other) noexcept
{
    PuzzleTiles.reserve(64);
    *this = other;
}

//----------------------------------
// SudokuBoard Operators
//----------------------------------

GameBoard& GameBoard::operator = (const GameBoard& other) noexcept
{
    this->BoardInitialized = other.BoardInitialized;
    this->BoardOccurences = other.BoardOccurences;

    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            this->BoardTiles[row][col].TileNumber    = other.BoardTiles[row][col].TileNumber;
            this->BoardTiles[row][col].Cell          = other.BoardTiles[row][col].Cell;
            this->BoardTiles[row][col].Row           = other.BoardTiles[row][col].Row;
            this->BoardTiles[row][col].Column        = other.BoardTiles[row][col].Column;
            this->BoardTiles[row][col].Pencilmarks   = other.BoardTiles[row][col].Pencilmarks;
            this->BoardTiles[row][col].InitializeTileOccurences(this->BoardOccurences);
        }
    }

    // Do not create new puzzle tiles if the former does not have one!
    if (other.PuzzleTiles.size() == 0) {
        return *this;
    }

    // Create new puzzle tiles because the pointers will be pointing to different objects!
    this->CreatePuzzleTiles();
    return *this;
}

bool GameBoard::operator== (const GameBoard& other) const noexcept
{
    for (int row = 0; row < 9; ++row)
        for (int col = 0; col < 9; ++col)
            if (this->GetTile(row, col).TileNumber != other.GetTile(row, col).TileNumber)
                return false;

    return true;
}

//----------------------------------
// SudokuBoard Helper Queries
//----------------------------------

bool GameBoard::IsBoardCompleted() const noexcept
{
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            if (!BoardTiles[row][col].IsTileFilled())
                return false;
        }
    }

    return true;
}

BoardTile& GameBoard::GetTile(int row, int column) noexcept
{
    return BoardTiles[row][column];
}

const BoardTile& GameBoard::GetTile(int row, int column) const noexcept
{
    return BoardTiles[row][column];
}

std::bitset<9>& GameBoard::GetTilePencilMarks(int row, int column) noexcept
{
    assert(!BoardTiles[row][column].IsTileFilled());

    return BoardTiles[row][column].Pencilmarks;
}

bool GameBoard::IsTileCandidateUsed(int row, int column, int bit_number) noexcept
{
    assert(!BoardTiles[row][column].IsTileFilled());

    return BoardTiles[row][column].Pencilmarks[bit_number];
}

bool GameBoard::IsTileFilled(int row, int column) noexcept
{
    return BoardTiles[row][column].IsTileFilled();
}

//----------------------------------
// SudokuBoard Basic Functions
//----------------------------------

bool GameBoard::CreateSudokuBoard(const std::array<std::array<int, 9>, 9>& sudoku_board, bool create_puzzletiles_vec) noexcept
{
    this->ClearSudokuBoard();
    BoardInitialized = this->CreateBoardOccurences(sudoku_board);

    for (int row = 0; row < 9; ++row)
        for (int col = 0; col < 9; ++col)
            BoardTiles[row][col].Initialize(sudoku_board[row][col], row, col, BoardOccurences); 
    
    if (create_puzzletiles_vec)
        this->CreatePuzzleTiles();

    return BoardInitialized;
}

bool GameBoard::CreateBoardOccurences(const std::array<std::array<int, 9>, 9>& board) noexcept
{
    BoardOccurences.ResetAll();

    bool invalid_board = false;
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            if (board[row][col] < 0 || board[row][col] > 9) {
                return false;
            }
            if (board[row][col] != 0) {
                int bit_number = board[row][col] - 1;
                if (BoardOccurences.GetTileOccurences(row, col)[bit_number])
                    invalid_board = true;
                BoardOccurences.SetCellNumber(row, col, bit_number);
            }
        }
    }

    return !invalid_board;
}

void GameBoard::ClearSudokuBoard() noexcept
{
    for (auto& row_tiles : BoardTiles) {
        for (auto& tile : row_tiles) {
            tile.Clear();
        }
    }

    BoardInitialized = false;
    BoardOccurences.ResetAll();
    PuzzleTiles.clear();
}

void GameBoard::CreatePuzzleTiles() noexcept
{
    PuzzleTiles.clear();
    for (auto& row_tiles : BoardTiles)
        for (auto& tile : row_tiles)
            if (!tile.IsTileFilled())
                PuzzleTiles.push_back(&tile);
}

void GameBoard::UpdateBoardOccurences() noexcept
{
    BoardOccurences.ResetAll();

    for (int row = 0; row < 9; ++row)
        for (int col = 0; col < 9; ++col)
            if (BoardTiles[row][col].IsTileFilled())
                BoardOccurences.SetCellNumber(row, col, BoardTiles[row][col].TileNumber - 1);
}

void GameBoard::UpdateBoardOccurences(int row, int col) noexcept
{
    int cell = sdq::helpers::GetCellBlock(row, col);
    auto [min_row, max_row, min_col, max_col] = sdq::helpers::GetMinMaxRowColumnFromCell(cell);

    for (int cell_row = min_row; cell_row < max_row; ++cell_row)
        for (int cell_col = min_col; cell_col < max_col; ++cell_col)
            if (BoardTiles[cell_row][cell_col].IsTileFilled())
                BoardOccurences.SetCellNumber(cell_row, cell_col, BoardTiles[cell_row][cell_col].TileNumber - 1);


    for (int new_row = 0; new_row < 9; ++new_row) {
        if (new_row >= min_row && new_row < max_row)
            continue;
        if (!BoardTiles[new_row][col].IsTileFilled())
            continue;
        BoardOccurences.SetCellNumber(new_row, col, BoardTiles[new_row][col].TileNumber - 1);
    }

    for (int new_col = 0; new_col < 9; ++new_col) {
        if (new_col >= min_col && new_col < max_col)
            continue;
        if (!BoardTiles[row][new_col].IsTileFilled())
            continue;
        BoardOccurences.SetCellNumber(row, new_col, BoardTiles[row][new_col].TileNumber - 1);
    }
}

//------------------------------------------------------
// SudokuBoard Backtracking Sudoku Solver Queries
//------------------------------------------------------

BoardTile* GameBoard::FindNextEmptyPosition(int row, int col) noexcept
{
    while (row != 9) {
        if (!this->IsTileFilled(row, col)) {
            return &BoardTiles[row][col];
        }
        row = sdq::helpers::GetNextRow(row, col);
        col = sdq::helpers::GetNextCol(col);
    }

    return nullptr;
}

BoardTile* GameBoard::FindNextEmptyPosition() noexcept
{
    for (auto& puzzle_tile : PuzzleTiles) {
        if (!puzzle_tile->IsTileFilled()) {
            return puzzle_tile;
        }
    }

    return nullptr;
}

BoardTile* GameBoard::FindLowestMRV() noexcept
{
    int lowest_mrv = -1;
    int highest_mrv_index = 0;
    for (int idx = 0; idx < PuzzleTiles.size(); ++idx) {
        if (PuzzleTiles[idx]->IsTileFilled()) {
            continue;
        }
        
        const auto& current_mrv_sum = PuzzleTiles[idx]->GetTileOccurences();
        const int& count = current_mrv_sum.count();
        if (lowest_mrv < count) {
            lowest_mrv  = count;
            highest_mrv_index = idx;
        }
    }

    if (lowest_mrv == -1) {
        return nullptr;
    }

    return PuzzleTiles[highest_mrv_index];
}

//------------------------------------------------
// SudokuBoard Pencilmark Functions
//------------------------------------------------

void GameBoard::UpdateRemovePencilMarks() noexcept
{
    for (auto& row_tiles : BoardTiles) {
        for (auto& tile : row_tiles) {
            if (!tile.IsTileFilled()) {
                tile.RemovePencilmarks();
            }
        }
    }
}

void GameBoard::UpdateReapplyPencilMarks() noexcept
{
    for (auto& row_tiles : BoardTiles) {
        for (auto& tile : row_tiles) {
            if (!tile.IsTileFilled()) {
                tile.ReapplyPencilmarks();
            }
        }
    }
}

void GameBoard::ResetAllPencilMarks() noexcept
{
    for (auto& row_tiles : BoardTiles) {
        for (auto& tile : row_tiles) {
            if (!tile.IsTileFilled()) {
                tile.ResetPencilmarks();
            }
        }
    }
}

bool GameBoard::UpdateRowPencilMarks(int row, int bit_number, const std::vector<int>& exempted_cells) noexcept
{
    int count = 0;

    for (int col = 0; col < 9; ++col) {
        if (BoardTiles[row][col].IsTileFilled() || BoardTiles[row][col].Pencilmarks[bit_number] || std::find_if(exempted_cells.begin(), exempted_cells.end(), [&](const int& cell_num) { return cell_num == BoardTiles[row][col].Cell; }) != exempted_cells.end()) {
            continue;
        }

        BoardTiles[row][col].Pencilmarks.set(bit_number);
        ++count;
    }

    return count != 0;
}

bool GameBoard::UpdateRowPencilMarks(int row, int bit_number, const std::vector<BoardTile*>& exempted_tiles) noexcept
{
    int count = 0;

    for (int col = 0; col < 9; ++col) {
        if (BoardTiles[row][col].IsTileFilled() || BoardTiles[row][col].Pencilmarks[bit_number] || std::find_if(exempted_tiles.begin(), exempted_tiles.end(), [&](const BoardTile* tile) { return *tile == BoardTiles[row][col]; }) != exempted_tiles.end()) {
            continue;
        }

        BoardTiles[row][col].Pencilmarks.set(bit_number);
        ++count;

        // Checks for assertions if you removed the last candidate
        assert(!BoardTiles[row][col].Pencilmarks.all());
    }

    return count != 0;
}

bool GameBoard::UpdateColumnPencilMarks(int col, int bit_number, const std::vector<int>& exempted_cells) noexcept
{
    int count = 0;

    for (int row = 0; row < 9; ++row) {
        if (BoardTiles[row][col].IsTileFilled() || BoardTiles[row][col].Pencilmarks[bit_number] || std::find_if(exempted_cells.begin(), exempted_cells.end(), [&](const int& cell_num) { return cell_num == BoardTiles[row][col].Cell; }) != exempted_cells.end()) {
            continue;
        }

        BoardTiles[row][col].Pencilmarks.set(bit_number);
        ++count;
    }

    return count != 0;
}

bool GameBoard::UpdateColumnPencilMarks(int col, int bit_number, const std::vector<BoardTile*>& exempted_tiles) noexcept
{
    int count = 0;

    for (int row = 0; row < 9; ++row) {
        if (BoardTiles[row][col].IsTileFilled() || BoardTiles[row][col].Pencilmarks[bit_number] || std::find_if(exempted_tiles.begin(), exempted_tiles.end(), [&](const BoardTile* tile) { return *tile == BoardTiles[row][col]; }) != exempted_tiles.end()) {
            continue;
        }

        BoardTiles[row][col].Pencilmarks.set(bit_number);
        ++count;

        // Checks for assertions if you removed the last candidate
        assert(!BoardTiles[row][col].Pencilmarks.all());
    }

    return count != 0;
}

bool GameBoard::UpdateCellPencilMarks(int cell, int bit_number, const std::vector<BoardTile*>& exempted_tiles) noexcept
{
    size_t count = 0;
    const auto& [min_row, max_row, min_col, max_col] = sdq::helpers::GetMinMaxRowColumnFromCell(cell);

    for (int row = min_row; row < max_row; ++row) {
        for (int col = min_col; col < max_col; ++col) {
            if (BoardTiles[row][col].IsTileFilled() || BoardTiles[row][col].Pencilmarks[bit_number] || std::find_if(exempted_tiles.begin(), exempted_tiles.end(), [&, this](const BoardTile* tile) {return *tile == BoardTiles[row][col]; }) != exempted_tiles.end()) {
                continue;
            }

            BoardTiles[row][col].Pencilmarks.set(bit_number);
            ++count;
        }
    }

    return count != 0;
}

bool GameBoard::UpdateCellPencilMarks(int cell, int bit_number, int exempted_line_idx, int row_or_column) noexcept
{
    size_t count = 0;
    const auto& [min_row, max_row, min_col, max_col] = sdq::helpers::GetMinMaxRowColumnFromCell(cell);

    for (int row = min_row; row < max_row; ++row) {
        for (int col = min_col; col < max_col; ++col) {
            if (BoardTiles[row][col].IsTileFilled() || BoardTiles[row][col].Pencilmarks[bit_number] || (row_or_column ? row == exempted_line_idx : col == exempted_line_idx)) {
                continue;
            }

            BoardTiles[row][col].Pencilmarks.set(bit_number);
            ++count;
        }
    }

    return count != 0;
}

bool GameBoard::UpdateCellLinePencilMarks(int cell, int bit_number, int line_idx, int row_or_column) noexcept
{
    size_t count = 0;
    const auto& [min_row, max_row, min_col, max_col] = sdq::helpers::GetMinMaxRowColumnFromCell(cell);

    if (row_or_column == RowOrColumn_Row) {
        for (int col = min_col; col < max_col; ++col) {
            if (BoardTiles[line_idx][col].IsTileFilled() || BoardTiles[line_idx][col].Pencilmarks[bit_number]) {
                continue;
            }

            BoardTiles[line_idx][col].Pencilmarks.set(bit_number);
            ++count;
        }
    }
    else {
        for (int row = min_row; row < max_row; ++row) {
            if (BoardTiles[row][line_idx].IsTileFilled() || BoardTiles[row][line_idx].Pencilmarks[bit_number]) {
                continue;
            }

            BoardTiles[row][line_idx].Pencilmarks.set(bit_number);
            ++count;
        }
    }

    return count != 0;
}

bool GameBoard::UpdateTilePencilMarks(const std::vector<BoardTile*>& sudoku_tiles, const std::vector<int> exempted_numbers) noexcept
{
    size_t count = 0;

    for (auto& tile : sudoku_tiles) {
        for (int bit_num = 0; bit_num < 9; ++bit_num) {
            if (tile->Pencilmarks[bit_num] || std::find_if(exempted_numbers.begin(), exempted_numbers.end(), [&](int num){ return num == bit_num; }) != exempted_numbers.end()) {
                continue;
            }

            ++count;
            tile->Pencilmarks.set(bit_num);
        }
    }

    return count != 0;
}

bool GameBoard::IsCandidatePresentInTheSameCell(int cell, int bit_number, const std::vector<BoardTile*> exempted_tiles) noexcept
{
    const auto& [min_row, max_row, min_col, max_col] = sdq::helpers::GetMinMaxRowColumnFromCell(cell);

    for (int row = min_row; row < max_row; ++row) {
        for (int col = min_col; col < max_col; ++col) {
            if (BoardTiles[row][col].IsTileFilled() || std::find_if(exempted_tiles.begin(), exempted_tiles.end(), [&](const BoardTile* tile) { return *tile == BoardTiles[row][col]; }) != exempted_tiles.end()) {
                continue;
            }

            if (!BoardTiles[row][col].Pencilmarks[bit_number]) {
                return true;
            }
        }
    }
    
    return false;
}

bool GameBoard::IsCandidatePresentInTheSameLine(int line_index, int bit_number, int row_or_column, const std::vector<BoardTile*> exempted_tiles) noexcept
{
    if (row_or_column == RowOrColumn_Row) {
        for (int col = 0; col < 9; ++col) {
            if (BoardTiles[line_index][col].IsTileFilled() || std::find_if(exempted_tiles.begin(), exempted_tiles.end(), [&](const BoardTile* tile) { return *tile == BoardTiles[line_index][col]; }) != exempted_tiles.end()) {
                continue;
            }

            if (!BoardTiles[line_index][col].Pencilmarks[bit_number]) {
                return true;
            }
        }
    }
    else {
        for (int row = 0; row < 9; ++row) {
            if (BoardTiles[row][line_index].IsTileFilled() || std::find_if(exempted_tiles.begin(), exempted_tiles.end(), [&](const BoardTile* tile) { return *tile == BoardTiles[row][line_index]; }) != exempted_tiles.end()) {
                continue;
            }

            if (!BoardTiles[row][line_index].Pencilmarks[bit_number]) {
                return true;
            }
        }
    }

    return false;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
// TurnLog CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------

TurnLog::TurnLog() noexcept :
    UndoPosition(0)
{
    TilesUsed.reserve(100);
}

void TurnLog::Add(int _row, int _col, int prev_num, int next_num, const std::bitset<9>& prev_pm, const std::bitset<9>& next_pm) noexcept
{
    if (UndoPosition < TilesUsed.size())
        TilesUsed.resize(UndoPosition);

    TilesUsed.push_back(TurnTile(_row, _col, prev_num, next_num, prev_pm, next_pm));
    UndoPosition = TilesUsed.size();
}

void TurnLog::Undo() noexcept
{
    if (TilesUsed.empty() || UndoPosition == 0)
        return;

    UndoPosition--;
}

void TurnLog::Redo() noexcept
{
    if (UndoPosition == TilesUsed.size())
        return;

    UndoPosition++;
}

void TurnLog::Reset() noexcept
{
    TilesUsed.clear();
    UndoPosition = 0;
}

const TurnLog::TurnTile* TurnLog::GetUndoTile() const noexcept
{
    return TilesUsed.empty() || UndoPosition == 0 ? nullptr : &TilesUsed[UndoPosition - 1];
}

const TurnLog::TurnTile* TurnLog::GetRedoTile() const noexcept
{
    return UndoPosition == TilesUsed.size() ? nullptr : &TilesUsed[UndoPosition];
}

bool TurnLog::CanUndo() const noexcept
{
    return !TilesUsed.empty() && UndoPosition != 0;
}

bool TurnLog::CanRedo() const noexcept
{
    return UndoPosition != TilesUsed.size();
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Time CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------------
// GameContext CLASS
//--------------------------------------------------------------------------------------------------------------------------------

Instance::Instance() : GameDifficulty(2), RandomDifficulty(0)
{
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    GameRNG.seed(seed);
}

bool Instance::CreateSudoku(const std::array<std::array<int, 9>, 9>& board) noexcept
{
    if (!SolutionBoard.CreateSudokuBoard(board))
        return false;

    PuzzleBoard = SolutionBoard;

    if (!sdq::solvers::Solve(SolutionBoard, SolveMethod_MRV))
        return false;

    GameDifficulty   = SudokuDifficulty_Random;
    RandomDifficulty = sdq::utils::CheckPuzzleDifficulty(PuzzleBoard);
    GameTurnLogs.Reset();

    return true;
}

bool Instance::LoadSudokuSave(const char* filepath) noexcept
{
    std::ifstream sudoku_save(filepath, std::ios::binary);
    if (!sudoku_save.good())
        return false;

    boost::archive::binary_iarchive iarchive(sudoku_save);
    try {
        this->GameDifficulty = SudokuDifficulty_Random;
        iarchive & this->RandomDifficulty;

        std::array<std::array<int, 9>, 9> solutionboard_numbers;
        for (size_t row = 0; row < 9; ++row) {
            for (size_t col = 0; col < 9; ++col) {
                iarchive & solutionboard_numbers[row][col];
                if (solutionboard_numbers[row][col] < 0 || solutionboard_numbers[row][col] > 9)
                    throw;
            }
        }

        this->SolutionBoard.CreateSudokuBoard(solutionboard_numbers, false);

        std::array<std::array<int, 9>, 9> puzzleboard_numbers;
        std::array<std::bitset<9>, 81>    puzzleboard_pencilmarks;
        for (size_t row = 0; row < 9; ++row) {
            for (size_t col = 0; col < 9; ++col) {
                iarchive & puzzleboard_numbers[row][col];
                iarchive & puzzleboard_pencilmarks[(row * 9) + col];

                if (puzzleboard_numbers[row][col] < 0 || puzzleboard_numbers[row][col] > 9)
                    throw;
            }
        }

        this->PuzzleBoard.CreateSudokuBoard(puzzleboard_numbers, false);
        for (size_t row = 0; row < 9; ++row)
            for (size_t col = 0; col < 9; ++col)
                this->PuzzleBoard.GetTile(row, col).Pencilmarks = puzzleboard_pencilmarks[(row * 9) + col];

        int puzzle_row = 0;
        int puzzle_col = 0;
        while (true) {
            try {
                iarchive & puzzle_row;
                iarchive & puzzle_col;
                this->PuzzleBoard.PuzzleTiles.push_back(&this->PuzzleBoard.GetTile(puzzle_row, puzzle_col));
            }
            catch (const std::exception& e) {
                break;
            }
        }
    }
    catch (const std::exception&) {
        return false;
    }

    GameTurnLogs.Reset();

    return true;
}

bool Instance::SaveCurrentProgress(const char* filepath) const noexcept
{
    std::ofstream ofile(filepath, std::ios::binary);
    if (!ofile.good())
        return false;

    boost::archive::binary_oarchive archive(ofile);
    // Archive the difficulty
    archive& this->GetBoardDifficulty();

    // Archive the solution board
    for (auto& row_tile : this->SolutionBoard.BoardTiles)
        for (auto& tile : row_tile)
            archive& tile.TileNumber;

    // Archive the puzzle board
    for (auto& row_tile : this->PuzzleBoard.BoardTiles) {
        for (auto& tile : row_tile) {
            archive& tile.TileNumber;
            archive& tile.Pencilmarks;
        }
    }

    for (auto& puzzle_tile : this->PuzzleBoard.PuzzleTiles) {
        archive& puzzle_tile->Row;
        archive& puzzle_tile->Column;
    }

    return true;
}

SudokuDifficulty Instance::LoadDifficultyFromSaveFile(const char* filepath) noexcept
{
    std::ifstream ifile(filepath, std::ios::binary);
    if (!ifile.good())
        return SudokuDifficulty_Random;

    boost::archive::binary_iarchive iarchive(ifile);
    SudokuDifficulty difficulty = SudokuDifficulty_Random;
    try {
        iarchive & difficulty;
    }
    catch (const std::exception&) {
        return SudokuDifficulty_Random;
    }

    return difficulty;
}

bool Instance::CreateSudoku(SudokuDifficulty game_difficulty) noexcept
{
    this->InitializeGameParameters(game_difficulty);  // Initialize important game parameters for creating a sudoku puzzle
    do {
        if (!this->CreateCompleteBoard())
            return false;

        if (this->GeneratePuzzle())
            break;
    } while (true);

    GameTurnLogs.Reset();

    return true;
}

void Instance::InitializeGameParameters(SudokuDifficulty game_difficulty) noexcept
{
    this->GameDifficulty = game_difficulty;
    switch (game_difficulty)
    {
    case SudokuDifficulty_Random: {
        std::uniform_int_distribution<> int_distrib(48, 64);
        MaxRemovedTiles = int_distrib(GameRNG);
        break;
    }
    case SudokuDifficulty_Easy:
        MaxRemovedTiles = 42;
        break;
    case SudokuDifficulty_Normal:
        MaxRemovedTiles = 48;
        break;
    case SudokuDifficulty_Insane:
        MaxRemovedTiles = 54;
        break;
    case SudokuDifficulty_Diabolical:
        MaxRemovedTiles = 64;
        break;
    default:
        MaxRemovedTiles = 52;
        break;
    }
}

void Instance::ClearAllBoards() noexcept
{
    SolutionBoard.ClearSudokuBoard();
}

bool Instance::CreateCompleteBoard() noexcept
{
    SolutionBoard.ClearSudokuBoard();

    static std::array<int, 9> random_numbers = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto fill_diagonal_cells = [this](int start_row, int end_row, int start_col, int end_col) {
        std::shuffle(random_numbers.begin(), random_numbers.end(), GameRNG);
        int num_idx = 0;
        for (int row = start_row; row < end_row; ++row) {
            for (int col = start_col; col < end_col; ++col) {
                SolutionBoard.BoardTiles[row][col].SetTileNumber(random_numbers[num_idx]);
                num_idx++;
            }
        }
    };

    fill_diagonal_cells(0, 3, 0, 3); // We will fill the three      x o o
    fill_diagonal_cells(3, 6, 3, 6); // non-connecting diagonal     o x o
    fill_diagonal_cells(6, 9, 6, 9); // cells of the sudoku board   o o x

    std::shuffle(random_numbers.begin(), random_numbers.end(), GameRNG);
    if (!sdq::utils::FillSudoku(SolutionBoard, random_numbers))
        return false;

    SolutionBoard.BoardInitialized = true;
    return true;
}

bool Instance::GeneratePuzzle() noexcept
{
    PuzzleBoard = SolutionBoard;

    constexpr size_t max_number_of_tiles = 81;
    static std::array<std::pair<int, int>, max_number_of_tiles> tiles_to_be_removed = { { {0, 0},{0, 1},{0, 2},{0, 3},{0, 4},{0, 5},{0, 6},{0, 7},{0, 8},{1, 0},{1, 1},{1, 2},{1, 3},{1, 4},{1, 5},{1, 6},{1, 7},{1, 8},{2, 0},{2, 1},{2, 2},{2, 3},{2, 4},{2, 5},{2, 6},{2, 7},{2, 8},{3, 0},{3, 1},{3, 2},{3, 3},{3, 4},{3, 5},{3, 6},{3, 7},{3, 8},{4, 0},{4, 1},{4, 2},{4, 3},{4, 4},{4, 5},{4, 6},{4, 7},{4, 8},{5, 0},{5, 1},{5, 2},{5, 3},{5, 4},{5, 5},{5, 6},{5, 7},{5, 8},{6, 0},{6, 1},{6, 2},{6, 3},{6, 4},{6, 5},{6, 6},{6, 7},{6, 8},{7, 0},{7, 1},{7, 2},{7, 3},{7, 4},{7, 5},{7, 6},{7, 7},{7, 8},{8, 0},{8, 1},{8, 2},{8, 3},{8, 4},{8, 5},{8, 6},{8, 7},{8, 8} } };
    // Shuffle the array so that it would not just remove tiles from the top left to bottom right
    // because there is also a stop flag when a certain number of removed tiles is reached
    std::shuffle(tiles_to_be_removed.begin(), tiles_to_be_removed.end(), GameRNG);

    int removed_tiles = 0;
    for (int i = 0; i < max_number_of_tiles && removed_tiles < MaxRemovedTiles; ++i) {
        const auto& row = tiles_to_be_removed[i].first;
        const auto& col = tiles_to_be_removed[i].second;
        auto tile_num = PuzzleBoard.BoardTiles[row][col].TileNumber;
        if (tile_num != 0) {
            // Removes the tile
            PuzzleBoard.BoardTiles[row][col].ResetTileNumber();

            // Put back the removed tile if the board does not have a unique solution
            if (!sdq::utils::IsUniqueBoard(PuzzleBoard)) {
                PuzzleBoard.BoardTiles[row][col].SetTileNumber(tile_num);
                continue;
            }
            removed_tiles++;
        }
    }

    // Create the neccesary puzzle tiles. Needed for solving the puzzle if someone wanted to, although there is already a solution
    PuzzleBoard.CreatePuzzleTiles();
    // Create the neccesary pencil marks of each tiles. Needed especially for most sudoku players
    PuzzleBoard.UpdateRemovePencilMarks();

    if (GameDifficulty == SudokuDifficulty_Random)
        RandomDifficulty = sdq::utils::CheckPuzzleDifficulty(PuzzleBoard);
    else if (sdq::utils::CheckPuzzleDifficulty(PuzzleBoard) != GameDifficulty)
        return false;

    return true;
}

//----------------------------------------------------------------------
// Sudoku GETTERS
//----------------------------------------------------------------------

const GameBoard* Instance::GetPuzzleBoard() const noexcept
{
    return &PuzzleBoard;
}

SudokuDifficulty Instance::GetBoardDifficulty() const noexcept
{
    return GameDifficulty == SudokuDifficulty_Random ? RandomDifficulty : GameDifficulty;
}

const GameBoard* Instance::GetSolutionBoard() const noexcept
{
    return &SolutionBoard;
}

const TurnLog* Instance::GetTurnLogs() const noexcept
{
    return &GameTurnLogs;
}

//----------------------------------------------------------------------
// Sudoku SETTERS
//----------------------------------------------------------------------

bool Instance::SetTile(int row, int col, int number) noexcept
{
    auto& input_tile = PuzzleBoard.GetTile(row, col);

    // In case the input 
    if (number == input_tile.TileNumber)
        return false;

    GameTurnLogs.Add(input_tile.Row, input_tile.Column, input_tile.TileNumber, number, input_tile.Pencilmarks, input_tile.Pencilmarks);

    // Checks if the input number is valid. If not, still register but return false
    const auto& occurences = input_tile.GetTileOccurences();
    input_tile.SetTileNumber(number);
    
    PuzzleBoard.UpdateBoardOccurences();

    if (number != 0)
        PuzzleBoard.UpdateRemovePencilMarks();
    else
        PuzzleBoard.UpdateReapplyPencilMarks();

    return true;
}

bool Instance::ResetTile(int row, int col) noexcept
{
    return SetTile(row, col, 0);
}

void Instance::ResetTurnLogs() noexcept
{
    GameTurnLogs.Reset();
}

void Instance::RemovePencilmark(int row, int col, int number) noexcept
{
    assert(number > 0 && number <= 9);
    auto& tile = PuzzleBoard.GetTile(row, col);
    auto previous_pm = tile.Pencilmarks;
    tile.Pencilmarks.set(number - 1);
    GameTurnLogs.Add(row, col, tile.TileNumber, tile.TileNumber, previous_pm, tile.Pencilmarks);
}

void Instance::AddPencilmark(int row, int col, int number) noexcept
{
    assert(number > 0 && number <= 9);
    auto& tile = PuzzleBoard.GetTile(row, col);
    auto previous_pm = tile.Pencilmarks;
    tile.Pencilmarks.reset(number - 1);
    GameTurnLogs.Add(row, col, tile.TileNumber, tile.TileNumber, previous_pm, tile.Pencilmarks);
}

void Instance::ResetAllPencilmarks() noexcept
{
    PuzzleBoard.ResetAllPencilMarks();
}

void Instance::ClearAllPencilmarks() noexcept
{
    for (auto& tile : PuzzleBoard.PuzzleTiles) {
        tile->Pencilmarks.reset();
        tile->Pencilmarks.flip();
    }
}

bool Instance::CheckPuzzleState() const noexcept
{
    if (!SolutionBoard.IsBoardCompleted())
        return false;

    // Checks if the solution board is the same as the puzzle board
    return SolutionBoard == PuzzleBoard;
}

bool Instance::IsValidTile(int row, int col) noexcept
{
    if (!PuzzleBoard.GetTile(row, col).IsTileFilled())
        return true;

    const auto [min_row, max_row, min_col, max_col] = sdq::helpers::GetMinMaxRowColumnFromCell(sdq::helpers::GetCellBlock(row, col));

    for (int new_row = min_row; new_row < max_row; ++new_row) {
        for (int new_col = min_col; new_col < max_col; ++new_col) {
            if (new_row == row && new_col == col)
                continue;

            if (PuzzleBoard.GetTile(new_row, new_col).TileNumber == PuzzleBoard.GetTile(row, col).TileNumber)
                return false;
        }
    }

    for (int new_row = 0; new_row < 9; ++new_row) {
        if (new_row >= min_row && new_row < max_row)
            continue;

        if (PuzzleBoard.GetTile(new_row, col).TileNumber == PuzzleBoard.GetTile(row, col).TileNumber)
            return false;
    }

    for (int new_col = 0; new_col < 9; ++new_col) {
        if (new_col >= min_col && new_col < max_col)
            continue;

        if (PuzzleBoard.GetTile(row, new_col).TileNumber == PuzzleBoard.GetTile(row, col).TileNumber)
            return false;
    }

    return true;
}

void Instance::UndoTurn() noexcept
{
    auto previous_turn_tile = GameTurnLogs.GetUndoTile();
    if (previous_turn_tile == nullptr)
        return;

    auto& input_tile = PuzzleBoard.GetTile(previous_turn_tile->Row, previous_turn_tile->Column);
    if (input_tile.TileNumber != previous_turn_tile->PreviousNumber) {
        input_tile.SetTileNumber(previous_turn_tile->PreviousNumber);
        PuzzleBoard.UpdateBoardOccurences(input_tile.Row, input_tile.Column);
        if (previous_turn_tile->PreviousNumber != 0)
            PuzzleBoard.UpdateRemovePencilMarks();
        else
            PuzzleBoard.UpdateReapplyPencilMarks();
    }

    input_tile.Pencilmarks = previous_turn_tile->PreviousPencilmark;

    GameTurnLogs.Undo();
}

void Instance::RedoTurn() noexcept
{
    auto next_turn_tile = GameTurnLogs.GetRedoTile();
    if (next_turn_tile == nullptr)
        return;

    auto& input_tile = PuzzleBoard.GetTile(next_turn_tile->Row, next_turn_tile->Column);

    if (input_tile.TileNumber == next_turn_tile->NextNumber)
        input_tile.Pencilmarks = next_turn_tile->NextPencilmark;
    else {
        input_tile.SetTileNumber(next_turn_tile->NextNumber);
        PuzzleBoard.UpdateBoardOccurences(input_tile.Row, input_tile.Column);
        if (next_turn_tile->NextNumber != 0)
            PuzzleBoard.UpdateRemovePencilMarks();
        else
            PuzzleBoard.UpdateReapplyPencilMarks();
    }

    GameTurnLogs.Redo();
}

}

namespace sdq::solvers
{

//----------------------------------------------------------------------------------------------------------------------------------------------
// Sudoku Solver Helper Functions
//----------------------------------------------------------------------------------------------------------------------------------------------

bool Solve(GameBoard& sudoku_board, SolveMethod method) noexcept
{
    switch (method)
    {
    case SolveMethod_BruteForce:
        return SolveBruteForce(sudoku_board);
    case SolveMethod_MRV:
        return SolveMRV(sudoku_board);
    case SolveMethod_Humanely:
        return SolveHumanely(sudoku_board);
    default:
        return false;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------
// Sudoku Solving Functions
//----------------------------------------------------------------------------------------------------------------------------------------------

bool SolveBruteForceEX(GameBoard& sudoku_board) noexcept
{
    auto* puzzle_tile = sudoku_board.FindNextEmptyPosition();

    if (puzzle_tile == nullptr) {
        return true;
    }

    const std::bitset<9>& occurences = puzzle_tile->GetTileOccurences();
    if (occurences.all()) {
        return false;
    }

    for (int bit_number = 0; bit_number < 9; ++bit_number) {
        if (occurences[bit_number]) {
            continue;
        }

        puzzle_tile->SetTileNumber(bit_number + 1);
        if (SolveBruteForceEX(sudoku_board)) {
            return true;
        }
        puzzle_tile->ResetTileNumber();
    }

    return false;
}

bool SolveBruteForce(GameBoard& sudoku_board) noexcept
{
    if (!sudoku_board.BoardInitialized) {
        return false;
    }

    return SolveBruteForceEX(sudoku_board);
}

bool SolveMRVEX(GameBoard& sudoku_board) noexcept
{
    auto* puzzle_tile = sudoku_board.FindLowestMRV();

    if (puzzle_tile == nullptr) {
        return true;
    }

    const std::bitset<9>& occurences = puzzle_tile->GetTileOccurences();
    if (occurences.all()) {
        return false;
    }

    for (int digit_idx = 0; digit_idx < 9; ++digit_idx) {
        if (occurences[digit_idx]) {
            continue;
        }

        puzzle_tile->SetTileNumber(digit_idx + 1);
        if (SolveMRVEX(sudoku_board)) {
            return true;
        }
        puzzle_tile->ResetTileNumber();
    }

    return false;
}

bool SolveMRV(GameBoard& sudoku_board) noexcept
{
    if (!sudoku_board.BoardInitialized) {
        return false;
    }

    return SolveMRVEX(sudoku_board);
}

bool SolveHumanelyEX(GameBoard& sudoku_board, size_t& difficulty_score) noexcept
{
    int used_techniques = 0;
    while (true) {
        if (size_t count = techs::FindSingleCandidates(sudoku_board)) {
            difficulty_score += count * 100;
            if (sudoku_board.IsBoardCompleted()) {
                return true;
            }
            sudoku_board.UpdateRemovePencilMarks();
            continue;
        }

        if (size_t count = techs::FindSinglePosition(sudoku_board)) {
            difficulty_score += count * 100;
            if (sudoku_board.IsBoardCompleted()) {
                return true;
            }
            sudoku_board.UpdateRemovePencilMarks();
            continue;
        }

        // These are functions for removing pencilmarks to decrease the number of possible numbers in tiles
        // As such, updating the pencilmarks is not needed
        if (size_t count = techs::FindCandidateLines(sudoku_board)) {
            difficulty_score += (used_techniques & UsedSudokuTechnique_CandidateLines) ? count * 200 : 350 + ((count - 1) * 200);
            used_techniques |= UsedSudokuTechnique_CandidateLines;
            continue;
        }

        if (size_t count = techs::FindIntersections(sudoku_board)) {
            difficulty_score += (used_techniques & UsedSudokuTechnique_Intersections) ? count * 500 : 800 + ((count - 1) * 500);
            used_techniques |= UsedSudokuTechnique_Intersections;
            continue;
        }

        {
            const auto& [pair_count, triple_count, quad_count] = techs::FindNakedTuples(sudoku_board);

            bool found = false;
            if (pair_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_NakedPair) ? pair_count * 500 : 750 + ((pair_count - 1) * 500);
                used_techniques |= UsedSudokuTechnique_NakedPair;
                found = true;
            }
            if (triple_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_NakedTriple) ? triple_count * 1400 : 2000 + ((triple_count - 1) * 1400);
                used_techniques |= UsedSudokuTechnique_NakedTriple;
                found = true;
            }
            if (quad_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_NakedQuad) ? quad_count * 4000 : 5000 + ((quad_count - 1) * 4000);
                used_techniques |= UsedSudokuTechnique_NakedQuad;
                found = true;
            }
            if (found)
                continue;
        }

        {
            const auto& [pair_count, triple_count, quad_count] = techs::FindHiddenTuples(sudoku_board);
            bool found = false;
            if (pair_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_HiddenPair) ? pair_count * 1200 : 1500 + ((pair_count - 1) * 1200);
                used_techniques |= UsedSudokuTechnique_HiddenPair;
                found = true;
            }
            if (triple_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_HiddenTriple) ? triple_count * 1600 : 2400 + ((triple_count - 1) * 1600);
                used_techniques |= UsedSudokuTechnique_HiddenTriple;
                found = true;
            }
            if (quad_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_HiddenQuad) ? quad_count * 5000 : 7000 + ((quad_count - 1) * 5000);
                used_techniques |= UsedSudokuTechnique_HiddenQuad;
                found = true;
            }
            if (found)
                continue;
        }

        if (size_t count = techs::FindYWings(sudoku_board)) {
            difficulty_score += (used_techniques & UsedSudokuTechnique_YWing) ? count * 2500 : 4000 + ((count - 1) * 2500);
            used_techniques |= UsedSudokuTechnique_YWing;
            continue;
        }

        {
            const auto& [xwing_count, swordfish_count, jellyfish_count] = techs::FindFishes(sudoku_board);
            bool found = false;
            if (xwing_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_XWing) ? swordfish_count * 2000 : 3000 + ((swordfish_count - 1) * 2000);
                used_techniques |= UsedSudokuTechnique_XWing;
                found = true;
            }
            if (swordfish_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_SwordFish) ? swordfish_count * 4000 : 5000 + ((swordfish_count - 1) * 4000);
                used_techniques |= UsedSudokuTechnique_SwordFish;
                found = true;
            }
            if (jellyfish_count > 0) {
                difficulty_score += (used_techniques & UsedSudokuTechnique_JellyFish) ? jellyfish_count * 5000 : 8000 + ((jellyfish_count - 1) * 5000);
                used_techniques |= UsedSudokuTechnique_JellyFish;
                found = true;
            }
            if (found)
                continue;
        }

        break;
    }

    return false;
}

bool SolveHumanely(GameBoard& sudoku_board, size_t* difficulty_score) noexcept
{
    if (!sudoku_board.BoardInitialized) {
        return false;
    }

    if (difficulty_score == nullptr) {
        size_t new_difficulty_score = 0;
        return SolveHumanelyEX(sudoku_board, new_difficulty_score);
    }

    return SolveHumanelyEX(sudoku_board, *difficulty_score);
}

}

namespace sdq::utils
{
    
bool FillSudokuEX(GameBoard& sudoku_board, const std::array<int, 9>& random_numbers, const int row_start, const int col_start) noexcept
{
    auto* puzzle_tile = sudoku_board.FindNextEmptyPosition(row_start, col_start);

    if (puzzle_tile == nullptr) {
        return true;
    }

    const std::bitset<9>& occurences = puzzle_tile->GetTileOccurences();
    if (occurences.all()) {
        return false;
    }

    for (int idx = 0; idx < 9; ++idx) {
        auto digit = random_numbers[idx];
        if (occurences[digit - 1]) {
            continue;
        }

        puzzle_tile->SetTileNumber(digit);
        if (FillSudokuEX(sudoku_board, random_numbers, puzzle_tile->Row, puzzle_tile->Column)) {
            return true;
        }
        puzzle_tile->ResetTileNumber();
    }

    return false;
}

bool FillSudoku(GameBoard& sudoku_board, const std::array<int, 9>& random_numbers) noexcept
{
    return FillSudokuEX(sudoku_board, random_numbers, 0, 0);
}

void CountSolutions(GameBoard& sudoku_board, size_t& number_of_solutions, int const row_start, int const col_start) noexcept
{
    auto* puzzle_tile = sudoku_board.FindNextEmptyPosition(row_start, col_start);

    // end of board 
    if (puzzle_tile == nullptr) {
        ++number_of_solutions;
        return;
    }

    std::bitset<9> const occurences = puzzle_tile->GetTileOccurences();
    if (occurences.all()) {
        return;
    }

    for (int digit_idx = 0; digit_idx < 9 && number_of_solutions < 2; ++digit_idx) {
        if (occurences[digit_idx]) {
            continue;
        }

        puzzle_tile->SetTileNumber(digit_idx + 1);
        CountSolutions(sudoku_board, number_of_solutions, puzzle_tile->Row, puzzle_tile->Column);
        puzzle_tile->ResetTileNumber();
    }
}

bool IsUniqueBoard(GameBoard& sudoku_board) noexcept
{
    size_t number_of_solutions = 0;
    CountSolutions(sudoku_board, number_of_solutions, 0, 0);
    return number_of_solutions == 1;
}

void PrintPencilMarks(const GameBoard& sudoku_board) noexcept
{
    printf("-------------------------------------------------------\n");
    for (int row = 0; row < 27; ++row) {
        int actual_row = row / 3;
        int min_bit = row % 3 * 3;
        int max_bit = min_bit + 3;
        printf("| ");
        for (int col = 0; col < 9; ++col) {
            for (int bit_num = min_bit; bit_num < max_bit; ++bit_num) {
                sudoku_board.BoardTiles[actual_row][col].IsTileFilled() || sudoku_board.BoardTiles[actual_row][col].Pencilmarks[bit_num] ?
                    (std::cout << ' ') : (std::cout << bit_num + 1);
            }
            std::cout << " | ";
        }
        printf("\n");
        if (actual_row < (row + 1) / 3) {
            printf("-------------------------------------------------------\n");
        }
    }
}

void PrintBoard(const sdq::GameBoard& board) noexcept
{
    printf(" -----------------------------------\n");
    for (size_t row = 0; row < 9; ++row) {
        printf("|");
        for (size_t col = 0; col < 9; ++col) {
            printf(" %d |", board.BoardTiles[row][col].TileNumber);
        }
        printf("\n -----------------------------------\n");
    }
}

SudokuDifficulty CheckPuzzleDifficulty(const GameBoard& sudoku_board) noexcept
{
    size_t difficulty_score = 0;
    auto sudoku_board_copy = sudoku_board;
    sdq::solvers::SolveHumanelyEX(sudoku_board_copy, difficulty_score);

    if (difficulty_score < 5000 && sudoku_board_copy.IsBoardCompleted()) {
        return SudokuDifficulty_Easy;
    }
    if (difficulty_score > 5000 && difficulty_score < 12000 && sudoku_board_copy.IsBoardCompleted()) {
        return SudokuDifficulty_Normal;
    }
    if (difficulty_score > 12000 && difficulty_score < 24000 && sudoku_board_copy.IsBoardCompleted()) {
        return SudokuDifficulty_Insane;
    }
    if (!sudoku_board_copy.IsBoardCompleted()) {
        size_t blank_count = std::count_if(sudoku_board_copy.PuzzleTiles.begin(), sudoku_board_copy.PuzzleTiles.end(), [&](const BoardTile* tile) { return !tile->IsTileFilled(); });
        if (blank_count < 5 && difficulty_score < 5000) {
            return SudokuDifficulty_Easy;
        }
        if (blank_count < 10 && difficulty_score < 12000) {
            return SudokuDifficulty_Normal;
        }
        if (blank_count < 25 && difficulty_score < 24000) {
            return SudokuDifficulty_Insane;
        }
    }
    return SudokuDifficulty_Diabolical;
}

SudokuDifficulty CheckPuzzleDifficulty(GameBoard& sudoku_board) noexcept
{
    size_t difficulty_score = 0;
    size_t blank_count = 0;
    const bool puzzle_completed = sdq::solvers::SolveHumanelyEX(sudoku_board, difficulty_score);
    if (!puzzle_completed)
        blank_count = std::count_if(sudoku_board.PuzzleTiles.begin(), sudoku_board.PuzzleTiles.end(), [](const BoardTile* tile) { return !tile->IsTileFilled(); });
    for (auto& ptile : sudoku_board.PuzzleTiles)
        if (ptile->IsTileFilled())
            ptile->ResetTileNumber();
    for (auto& ptile : sudoku_board.PuzzleTiles)
        ptile->ResetPencilmarks();

    if (difficulty_score < 5000 && puzzle_completed)
        return SudokuDifficulty_Easy;
    if (difficulty_score > 5000 && difficulty_score < 12000 && puzzle_completed)
        return SudokuDifficulty_Normal;
    if (difficulty_score > 12000 && difficulty_score < 24000 && puzzle_completed)
        return SudokuDifficulty_Insane;
    if (!puzzle_completed) {
        if (blank_count < 5 && difficulty_score < 5000)
            return SudokuDifficulty_Easy;
        if (blank_count < 10 && difficulty_score < 12000)
            return SudokuDifficulty_Normal;
        if (blank_count < 25 && difficulty_score < 24000)
            return SudokuDifficulty_Insane;
    }
    return SudokuDifficulty_Diabolical;
}

std::optional<std::array<std::array<int, 9>, 9>> OpenSudokuFile(const char* filename) noexcept
{
    std::ifstream ifile(filename, std::ios::in);
    if (!ifile.good())
        return std::nullopt;

    std::array<std::array<int, 9>, 9> output_array = { 0 };
    std::string ifile_data;
    std::getline(ifile, ifile_data);

    // Checks the string for the proper size
    // It should be 81 for 9x9 grid sudoku
    if (ifile_data.size() != 81)
        return std::nullopt;

    for (size_t row = 0; row < 9; ++row) {
        for (size_t col = 0; col < 9; ++col) {
            int current_number = ifile_data[(row * 9) + col] - '0';
            if (current_number < 0 || current_number > 9)
                return std::nullopt;
            output_array[row][col] = current_number;
        }
    }

    ifile.close();

    return output_array;
}

bool CreateSudokuFile(const GameBoard& sudoku_board, const char* filepath) noexcept
{
    std::fstream new_file(filepath, std::ios::in | std::ios::out | std::ios::trunc);
    if (!new_file.good())
        return false;

    for (auto& row_tile : sudoku_board.BoardTiles) {
        for (auto& tile : row_tile) {
            new_file << tile.TileNumber;
        }
    }

    for (auto& ptile : sudoku_board.PuzzleTiles) {
        new_file.seekp((ptile->Row * 9) + ptile->Column);
        new_file.put('0');
    }

    new_file.close();
    return true;
}

}

namespace sdq::techs
{

//
size_t FindSinglePosition(GameBoard& sudoku_board) noexcept
{
    size_t count = 0;

    for (int cell = 0; cell < 9; ++cell) {
        const auto& [min_row, max_row, min_col, max_col] = helpers::GetMinMaxRowColumnFromCell(cell);
        for (int bit_num = 0; bit_num < 9; ++bit_num) {
            int bit_count = 0;
            BoardTile* single_position_tile = nullptr;
            for (int row = min_row; row < max_row; ++row) {
                for (int col = min_col; col < max_col; ++col) {
                    if (sudoku_board.GetTile(row, col).IsTileFilled()) {
                        continue;
                    }

                    if (!sudoku_board.IsTileCandidateUsed(row, col, bit_num)) {
                        ++bit_count;
                        single_position_tile = &sudoku_board.BoardTiles[row][col];
                    }
                }
            }
            if (bit_count == 1) {
                single_position_tile->SetTileNumber(bit_num + 1);
                ++count;
            }
        }
    }

    return count;
}
//
size_t FindSingleCandidates(GameBoard& sudoku_board) noexcept
{
    size_t count = 0;

    for (auto& tile : sudoku_board.PuzzleTiles) {
        if (tile->IsTileFilled()) {
            continue;
        }

        const std::bitset<9>& occurences = ~tile->Pencilmarks;
        if (occurences.count() != 1) {
            continue;
        }

        for (int bit_num = 0; bit_num < 9; ++bit_num) {
            if (!occurences[bit_num]) {
                continue;
            }

            tile->SetTileNumber(bit_num + 1);
            ++count;
            break;
        }
    }

    return count;
}
//
size_t FindCandidateLines(GameBoard& sudoku_board) noexcept
{
    size_t count = 0;

    auto candidate_lines_lambda = [&](std::array<std::bitset<9>, 3> total_line_bitset, int min_line_index, int cell_number, int row_or_column) {
        for (int bit_num = 0; bit_num < 9; ++bit_num) {
            int bit_count = 0;
            int line_idx = 0;
            for (int bitset_idx = 0; bitset_idx < 3; ++bitset_idx) {
                if (!total_line_bitset[bitset_idx][bit_num]) {
                    ++bit_count;
                    line_idx = bitset_idx + min_line_index;
                }
            }
            if (bit_count == 1) {
                if (row_or_column ? sudoku_board.UpdateRowPencilMarks(line_idx, bit_num, std::vector<int>({ cell_number }) ) : 
                                    sudoku_board.UpdateColumnPencilMarks(line_idx, bit_num, std::vector<int>({ cell_number }))) {
                    ++count;
                }
            }
        }
    };

    for (int cell = 0; cell < 9; ++cell) {
        const auto& [min_row, max_row, min_col, max_col] = helpers::GetMinMaxRowColumnFromCell(cell);
        {
            std::array<std::bitset<9>, 3> total_row_bitset = { 0, 0, 0 };
            for (int row = min_row; row < max_row; ++row) {
                std::bitset<9> row_bitset = 0;
                for (int col = min_col; col < max_col; ++col) {
                    if (!sudoku_board.GetTile(row, col).IsTileFilled()) {
                        row_bitset |= ~sudoku_board.GetTilePencilMarks(row, col);
                    }
                }
                total_row_bitset[min_row == 0 ? row : row % min_row] = ~row_bitset;
            }
            candidate_lines_lambda(total_row_bitset, min_row, cell, RowOrColumn_Row);
        }
        {
            std::array<std::bitset<9>, 3> total_col_bitset = { 0, 0, 0 };
            for (int col = min_col; col < max_col; ++col) {
                std::bitset<9> col_bitset = 0;
                for (int row = min_row; row < max_row; ++row) {
                    if (!sudoku_board.GetTile(row, col).IsTileFilled()) {
                        col_bitset |= ~sudoku_board.GetTilePencilMarks(row, col);
                    }
                }
                total_col_bitset[min_col ? col % min_col : col] = ~col_bitset;
            }
            candidate_lines_lambda(total_col_bitset, min_col, cell, RowOrColumn_Column);
        }
    }

    return count;
}
//
size_t FindIntersections(GameBoard& sudoku_board) noexcept
{
    size_t count = 0;

    auto intersection_lambda = [&](int line_index, const int row_or_column, const std::array<std::bitset<9>, 3> line_bitsets) {
        for (int bit_num = 0; bit_num < 9; ++bit_num) {
            int bit_count = 0;
            int intersection_cell_idx = 0;
            for (int arr_idx = 0; arr_idx < line_bitsets.size(); ++arr_idx) {
                if (!line_bitsets[arr_idx][bit_num]) {
                    bit_count++;
                    intersection_cell_idx = row_or_column ? arr_idx + ((line_index / 3) * 3) : (arr_idx * 3) + (line_index / 3); // col 1 + ((row / 3) * 3)
                }
            }
            if (bit_count == 1) {
                if (sudoku_board.UpdateCellPencilMarks(intersection_cell_idx, bit_num, line_index, row_or_column)) {
                    ++count;
                }
            }
        }
    };

    std::array<std::bitset<9>, 3> row_bitsets;
    for (int row = 0; row < 9; ++row) {
        row_bitsets = { 0, 0, 0 };
        for (auto& bits : row_bitsets) { bits.flip(); }
        for (int col = 0; col < 9; ++col) {
            if (sudoku_board.GetTile(row, col).IsTileFilled()) {
                continue;
            }

            int array_index = helpers::GetCellBlock(row, col) % 3;
            row_bitsets[array_index] &= sudoku_board.GetTilePencilMarks(row, col);
        }
        intersection_lambda(row, RowOrColumn_Row, row_bitsets);
    }

    std::array<std::bitset<9>, 3> col_bitsets;
    for (int col = 0; col < 9; ++col) {
        col_bitsets = { 0, 0, 0};
        for (auto& bits : col_bitsets) { bits.flip(); }
        for (int row = 0; row < 9; ++row) {
            if (sudoku_board.GetTile(row, col).IsTileFilled()) {
                continue;
            }

            int array_index = helpers::GetCellBlock(row, col) / 3;
            col_bitsets[array_index] &= sudoku_board.GetTilePencilMarks(row, col);
        }
        intersection_lambda(col, RowOrColumn_Column, col_bitsets);
    }

    return count;
}
//
std::tuple<size_t, size_t, size_t> FindNakedTuples(GameBoard& sudoku_board) noexcept
{
    size_t pair_count   = 0;
    size_t triple_count = 0;
    size_t quad_count   = 0;

    auto find_naked_tuple_lambda = [&](const std::vector<BoardTile*>& tuple_tiles) {
        if (tuple_tiles.size() == 2) {
            return tuple_tiles[0]->Pencilmarks == tuple_tiles[1]->Pencilmarks && ~(tuple_tiles[0]->Pencilmarks.count()) == 2 ? tuple_tiles[0]->Pencilmarks : ~std::bitset<9>(0);
        }

        int number_of_tuples = 0;
        for (int bit_number = 0; bit_number < 9; ++bit_number) {
            int bit_count = 0;
            for (auto& tile : tuple_tiles) {
                if (!tile->Pencilmarks[bit_number]) {
                    ++bit_count;
                }
            }
            if      (bit_count >= 2) { ++number_of_tuples; }
            else if (bit_count == 1) { return ~std::bitset<9>(0); }
        }

        if (number_of_tuples != tuple_tiles.size()) {
            return ~std::bitset<9>(0);
        }

        std::bitset<9> output_bitset = 0;
        for (auto& tile : tuple_tiles) {
            output_bitset |= ~tile->Pencilmarks;
        }
        return ~output_bitset;
    };

    {
        std::vector<BoardTile*> naked_tuple_tiles;
        naked_tuple_tiles.reserve(4);
        for (int cell = 0; cell < 9; ++cell) {
            const auto& [min_row, max_row, min_col, max_col] = helpers::GetMinMaxRowColumnFromCell(cell);
            auto naked_tuple_cell = [&](int tuple_number) {
                auto recursive_tuple_cell = [&](int row, int col, auto& recursive_function) {
                    do {
                        col++;
                        if (col == max_col) {
                            col = min_col;
                            ++row;
                            if (row == max_row) {
                                return;
                            }
                        }
                    } while (sudoku_board.IsTileFilled(row, col));

                    naked_tuple_tiles.push_back(&sudoku_board.GetTile(row, col));
                    if (naked_tuple_tiles.size() != tuple_number) {
                        recursive_function(row, col, recursive_function);
                        naked_tuple_tiles.pop_back();
                        return recursive_function(row, col, recursive_function);
                    }

                    const std::bitset<9>& resultant_lambda = find_naked_tuple_lambda(naked_tuple_tiles);
                    if (resultant_lambda.all()) {
                        naked_tuple_tiles.pop_back();
                        return recursive_function(row, col, recursive_function);
                    }

                    bool success  = false;
                    bool same_row = tuple_number != 4;
                    bool same_col = tuple_number != 4;
                    int row_index = naked_tuple_tiles[0]->Row;
                    int col_index = naked_tuple_tiles[0]->Column;
                    if (tuple_number != 4) {
                        // Checks if the naked tuples are in the same row or column
                        // This would mean that you would also remove the candidates on that same row or column
                        for (auto& tiles : naked_tuple_tiles) {
                            same_row = same_row && tiles->Row == row_index;
                            same_col = same_col && tiles->Column == col_index;
                        }
                    }
                    for (int bit_num = 0; bit_num < 9; ++bit_num) {
                        if (resultant_lambda[bit_num]) {
                            continue;
                        }

                        success = sudoku_board.UpdateCellPencilMarks(cell, bit_num, naked_tuple_tiles) || success;
                        if (same_row)
                            success = sudoku_board.UpdateRowPencilMarks(row_index, bit_num, naked_tuple_tiles) || success;
                        else if (same_col)
                            success = sudoku_board.UpdateColumnPencilMarks(col_index, bit_num, naked_tuple_tiles) || success;
                    }
                    if (success) {
                        switch (tuple_number)
                        {
                        case 2: pair_count++;   break;
                        case 3: triple_count++; break;
                        case 4: quad_count++;   break;
                        default: break;
                        }
                    }

                    naked_tuple_tiles.pop_back();
                    return recursive_function(row, col, recursive_function);
                };
                for (int row = min_row; row < max_row; ++row) {
                    for (int col = min_col; col < max_col; ++col) {
                        if (sudoku_board.BoardTiles[row][col].IsTileFilled()) {
                            continue;
                        }
                        
                        naked_tuple_tiles.push_back(&sudoku_board.BoardTiles[row][col]);
                        recursive_tuple_cell(row, col, recursive_tuple_cell);
                        naked_tuple_tiles.clear();
                    }
                }
            };

            naked_tuple_cell(2);
            naked_tuple_cell(3);
            naked_tuple_cell(4);
        }
    }

    {
        std::vector<BoardTile*> naked_tuple_tiles;
        naked_tuple_tiles.reserve(4);
        auto naked_tuple_lines = [&](int tuple_number, int line_idx, int row_or_column, const std::array<std::vector<BoardTile*>, 3>& tuple_tiles) {
            auto recursive_tuple_line = [&](int arr_idx, int vec_idx, auto& recursive_function) {
                do {
                    if (arr_idx == tuple_tiles.size())
                        return;
                    
                    vec_idx++;
                    if (tuple_tiles[arr_idx].empty() || vec_idx >= tuple_tiles[arr_idx].size()) {
                        arr_idx++;
                        vec_idx = -1;
                        continue;
                    }
                    break;
                } while (true);

                naked_tuple_tiles.push_back(tuple_tiles[arr_idx][vec_idx]);
                if (naked_tuple_tiles.size() != tuple_number) {
                    recursive_function(arr_idx, vec_idx, recursive_function);
                    naked_tuple_tiles.pop_back();
                    return recursive_function(arr_idx, vec_idx, recursive_function);
                }

                if (tuple_number != 4 && naked_tuple_tiles.back()->Cell == naked_tuple_tiles.front()->Cell) {
                    naked_tuple_tiles.pop_back();
                    return recursive_function(arr_idx, vec_idx, recursive_function);
                }

                const std::bitset<9>& resultant_lambda = find_naked_tuple_lambda(naked_tuple_tiles);
                if (resultant_lambda.all()) {
                    naked_tuple_tiles.pop_back();
                    return recursive_function(arr_idx, vec_idx, recursive_function);
                }

                bool success = false;
                for (int bit_num = 0; bit_num < 9; ++bit_num) {
                    if (resultant_lambda[bit_num]) {
                        continue;
                    }

                    success = (row_or_column ? sudoku_board.UpdateRowPencilMarks(line_idx, bit_num, naked_tuple_tiles) :
                                               sudoku_board.UpdateColumnPencilMarks(line_idx, bit_num, naked_tuple_tiles)) || success;
                }
                if (success) {
                    switch (tuple_number)
                    {
                    case 2: pair_count++;   break;
                    case 3: triple_count++; break;
                    case 4: quad_count++;   break;
                    default: break;
                    }
                }

                naked_tuple_tiles.pop_back();
                recursive_function(arr_idx, vec_idx, recursive_function);
            };

            for (int arr_idx = 0; arr_idx < tuple_tiles.size(); ++arr_idx) {
                for (int vec_idx = 0; vec_idx < tuple_tiles[arr_idx].size(); ++vec_idx) {
                    naked_tuple_tiles.push_back(tuple_tiles[arr_idx][vec_idx]);
                    recursive_tuple_line(arr_idx, vec_idx, recursive_tuple_line);
                    naked_tuple_tiles.clear();
                }
            }
        };

        // Naked Tuples in rows
        std::array<std::vector<BoardTile*>, 3> naked_tuples_row;
        for (int row = 0; row < 9; ++row) {
            for (auto& row_tiles : naked_tuples_row) {
                row_tiles.clear();
            }
            for (int col = 0; col < 9; ++col) {
                if (sudoku_board.IsTileFilled(row, col)) {
                    continue;
                }

                int array_index = sudoku_board.GetTile(row, col).Cell % 3;
                naked_tuples_row[array_index].push_back(&sudoku_board.GetTile(row, col));
            }
            naked_tuple_lines(2, row, RowOrColumn_Row, naked_tuples_row);
            naked_tuple_lines(3, row, RowOrColumn_Row, naked_tuples_row);
            naked_tuple_lines(4, row, RowOrColumn_Row, naked_tuples_row);
        }

        // Naked Tuples in columns
        std::array<std::vector<BoardTile*>, 3> naked_tuples_col;
        for (int col = 0; col < 9; ++col) {
            for (auto& row_tiles : naked_tuples_col) {
                row_tiles.clear();
            }
            for (int row = 0; row < 9; ++row) {
                if (sudoku_board.IsTileFilled(row, col)) {
                    continue;
                }

                int array_index = sudoku_board.GetTile(row, col).Cell / 3;
                naked_tuples_col[array_index].push_back(&sudoku_board.GetTile(row, col));
            }
            naked_tuple_lines(2, col, RowOrColumn_Column, naked_tuples_col);
            naked_tuple_lines(3, col, RowOrColumn_Column, naked_tuples_col);
            naked_tuple_lines(4, col, RowOrColumn_Column, naked_tuples_col);
        }
    }

    return { pair_count, triple_count, quad_count };
}
//
std::tuple<size_t, size_t, size_t> FindHiddenTuples(GameBoard& sudoku_board) noexcept
{
    size_t pair_count   = 0;
    size_t triple_count = 0;
    size_t quad_count   = 0;

    auto find_hidden_tuple_lambda = [&](const std::vector<BoardTile*>& hidden_tuples, int cell_or_line, int cell_or_line_idx, int row_or_column = RowOrColumn_Row) {
        std::vector<int> possible_hidden_tuple_numbers;
        possible_hidden_tuple_numbers.reserve(6);

        // This is to find the numbers that are more than appears more than twice
        for (int bit_number = 0; bit_number < 9; ++bit_number) {
            int bit_count = 0;
            for (auto& tile : hidden_tuples) {
                if (!tile->Pencilmarks[bit_number]) {
                    ++bit_count;
                }
            }
            if (bit_count >= 2) {
                possible_hidden_tuple_numbers.push_back(bit_number);
            }
        }

        // The size of hidden_tiles is the tuple_number i.e. size == 2 is hidden pair
        // The size of possibilities should be equal or higher than the tuple number
        if (possible_hidden_tuple_numbers.size() < hidden_tuples.size()) {
            return;
        }

        // Finding the actual hidden tuple numbers
        // They are the numbers that does not appear on the same cell or line(row or column)
        std::vector<int> actual_hidden_tuple_numbers;
        actual_hidden_tuple_numbers.reserve(hidden_tuples.size());
        for (auto& numbers : possible_hidden_tuple_numbers) {
            if (cell_or_line ? !sudoku_board.IsCandidatePresentInTheSameCell(cell_or_line_idx, numbers, hidden_tuples) : !sudoku_board.IsCandidatePresentInTheSameLine(cell_or_line_idx, numbers, row_or_column, hidden_tuples)) {
                actual_hidden_tuple_numbers.push_back(numbers);
            }
        }

        // If the size of the actual numbers is the same as the tuple number, then it's a hidden tuples!
        if (actual_hidden_tuple_numbers.size() == hidden_tuples.size()) {
            if (sudoku_board.UpdateTilePencilMarks(hidden_tuples, actual_hidden_tuple_numbers)) {
                switch (hidden_tuples.size())
                {
                case 2:  pair_count++;   break;
                case 3:  triple_count++; break;
                case 4:  quad_count++;   break;
                default: break;
                }
            }
        }
    };

    {   // Find the hidden tuples in cells
        std::vector<BoardTile*> hidden_tuple_tiles;
        hidden_tuple_tiles.reserve(4);
        for (int cell = 0; cell < 9; ++cell) {
            const auto& [min_row, max_row, min_col, max_col] = helpers::GetMinMaxRowColumnFromCell(cell);
            // Main lambda for finding hidden tuples in cells
            auto find_hidden_tuple_cell = [&](int tuple_number) {
                // Recursive lambda for populating possible hidden tuple tiles
                auto recursive_find_hidden_tuple_cell = [&](int row, int col, auto& self_function) {
                    do {
                        col++;
                        if (col == max_col) {
                            col = min_col;
                            ++row;
                            if (row == max_row) {
                                return;
                            }
                        }
                    } while (sudoku_board.IsTileFilled(row, col));

                    hidden_tuple_tiles.push_back(&sudoku_board.GetTile(row, col));
                    if (hidden_tuple_tiles.size() != tuple_number) {
                        self_function(row, col, self_function);
                        hidden_tuple_tiles.pop_back();
                        return self_function(row, col, self_function);
                    }

                    find_hidden_tuple_lambda(hidden_tuple_tiles, 1, cell);
                    hidden_tuple_tiles.pop_back();
                    self_function(row, col, self_function);
                };
                for (int row = min_row; row < max_row; ++row) {
                    for (int col = min_col; col < max_col; ++col) {
                        if (sudoku_board.IsTileFilled(row, col)) { 
                            continue; 
                        }

                        hidden_tuple_tiles.push_back(&sudoku_board.GetTile(row, col));
                        recursive_find_hidden_tuple_cell(row, col, recursive_find_hidden_tuple_cell);
                        hidden_tuple_tiles.clear();
                    }
                }
            };

            find_hidden_tuple_cell(2);
            find_hidden_tuple_cell(3);
            find_hidden_tuple_cell(4);
        }
    }

    {   // Find the hidden tuples in lines (row or column)
        std::vector<BoardTile*> hidden_tuple_tiles;
        hidden_tuple_tiles.reserve(4);
        // Start of finding hidden tuple in lines lambda
        auto find_hidden_tuple_lines = [&](int tuple_number, int line_idx, int row_or_column, const std::array<std::vector<BoardTile*>, 3>& tuple_tiles) {
            // Start of recursive tuple line lambda
            auto recursive_hidden_tuple_line = [&](int arr_idx, int vec_idx, auto& recursive_function) {
                do {
                    if (arr_idx == tuple_tiles.size())
                        return;

                    vec_idx++;
                    if (tuple_tiles[arr_idx].empty() || vec_idx >= tuple_tiles[arr_idx].size()) {
                        arr_idx++;
                        vec_idx = -1;
                        continue;
                    }
                    break;
                } while (true);

                hidden_tuple_tiles.push_back(tuple_tiles[arr_idx][vec_idx]);
                if (hidden_tuple_tiles.size() != tuple_number) {
                    recursive_function(arr_idx, vec_idx, recursive_function);
                    hidden_tuple_tiles.pop_back();
                    return recursive_function(arr_idx, vec_idx, recursive_function);
                }

                if (tuple_number != 4 && hidden_tuple_tiles.back()->Cell == hidden_tuple_tiles.front()->Cell) {
                    hidden_tuple_tiles.pop_back();
                    return recursive_function(arr_idx, vec_idx, recursive_function);
                }
                
                find_hidden_tuple_lambda(hidden_tuple_tiles, 0, line_idx, row_or_column);
                hidden_tuple_tiles.pop_back();
                recursive_function(arr_idx, vec_idx, recursive_function);
            };
            // End of recursive tuple line lambda

            for (int arr_idx = 0; arr_idx < tuple_tiles.size(); ++arr_idx) {
                for (int vec_idx = 0; vec_idx < tuple_tiles[arr_idx].size(); ++vec_idx) {
                    hidden_tuple_tiles.push_back(tuple_tiles[arr_idx][vec_idx]);
                    recursive_hidden_tuple_line(arr_idx, vec_idx, recursive_hidden_tuple_line);
                    hidden_tuple_tiles.clear();
                }
            }
        };
        // End of finding hidden tuple in lines lambda

        // Hidden Tuples in rows
        std::array<std::vector<BoardTile*>, 3> hidden_tuples_row;
        for (int row = 0; row < 9; ++row) {
            for (auto& row_tiles : hidden_tuples_row) {
                row_tiles.clear();
            }
            for (int col = 0; col < 9; ++col) {
                if (sudoku_board.IsTileFilled(row, col)) {
                    continue;
                }

                int array_index = helpers::GetCellBlock(row, col) % 3;
                hidden_tuples_row[array_index].push_back(&sudoku_board.GetTile(row, col));
            }
            find_hidden_tuple_lines(2, row, RowOrColumn_Row, hidden_tuples_row);
            find_hidden_tuple_lines(3, row, RowOrColumn_Row, hidden_tuples_row);
            find_hidden_tuple_lines(4, row, RowOrColumn_Row, hidden_tuples_row);
        }

        // Hidden Tuples in columns
        std::array<std::vector<BoardTile*>, 3> hidden_tuples_col;
        for (int col = 0; col < 9; ++col) {
            for (auto& row_tiles : hidden_tuples_col) {
                row_tiles.clear();
            }
            for (int row = 0; row < 9; ++row) {
                if (sudoku_board.IsTileFilled(row, col)) {
                    continue;
                }

                int array_index = helpers::GetCellBlock(row, col) / 3;
                hidden_tuples_col[array_index].push_back(&sudoku_board.GetTile(row, col));
            }
            find_hidden_tuple_lines(2, col, RowOrColumn_Column, hidden_tuples_col);
            find_hidden_tuple_lines(3, col, RowOrColumn_Column, hidden_tuples_col);
            find_hidden_tuple_lines(4, col, RowOrColumn_Column, hidden_tuples_col);
        }
    }

    return { pair_count, triple_count, quad_count };
}
//
size_t FindYWings(GameBoard& sudoku_board) noexcept
{
    size_t count = 0;

    // We first get the first possible pivot tile
    // A pivot tile should have 2 pencilmarks
    for (auto& pivot_tile_1 : sudoku_board.PuzzleTiles) {
        if (pivot_tile_1->IsTileFilled() || pivot_tile_1->Pencilmarks.count() != 7) {
            continue;
        }

        std::array<int, 2> pivot_numbers;
        int bit_count = 0;
        for (int bit_num = 0; bit_num < 9; ++bit_num) {
            if (!pivot_tile_1->Pencilmarks[bit_num]) {
                pivot_numbers[bit_count] = bit_num;
                bit_count++;
            }
        }

        auto find_y_wing_lambda = [&count, &sudoku_board, &pivot_tile_1, &pivot_numbers](int pincer_line_alignment, int line_idx, bool stop_finding_in_lines) {
            // Here we try to find another possible pivot tile that is aligned with the first pivot tile
            // If the pincer line alignment is in row, then we should traverse in column and vise versa
            // Finding the next possible pivot tile should start on the next cell!
            int starting_line_idx = (((pincer_line_alignment ? pivot_tile_1->Column : pivot_tile_1->Row) / 3) * 3) + 3;
            for (int opposing_line_idx = starting_line_idx; opposing_line_idx < 9; ++opposing_line_idx) {
                auto& pivot_tile_2 = pincer_line_alignment ? sudoku_board.GetTile(line_idx, opposing_line_idx) : sudoku_board.GetTile(opposing_line_idx, line_idx);

                // Same as with the first pivot tile, but the second pivot tile should not be equal to the first pivot tile
                if (pivot_tile_2.IsTileFilled() || pivot_tile_2.Pencilmarks.count() != 7 || pivot_tile_2.Pencilmarks == pivot_tile_1->Pencilmarks) {
                    continue;
                }

                // This assumes that the pivot_tile_1 is the real pivot tile and pivot_tile_2 is the first pincer tile
                // As for the opposite, then the pincer number would be the complementary_pivot_number
                int pincer_number;
                int complementary_pivot_number = -1;
                for (int bit_num = 0; bit_num < 9; ++bit_num) {
                    if (!pivot_tile_2.Pencilmarks[bit_num]) {
                        bit_num == pivot_numbers[0] ? complementary_pivot_number = pivot_numbers[1] :
                            bit_num == pivot_numbers[1] ? complementary_pivot_number = pivot_numbers[0] : pincer_number = bit_num;
                    }
                }

                if (complementary_pivot_number == -1) {
                    continue;
                }

                auto find_second_pincer_in_cell = [&count, &sudoku_board, &line_idx, &pincer_line_alignment](BoardTile& pivot_tile, BoardTile& pincer_tile_1, int pincer_number, int complementary_number) {
                    const int cell = helpers::GetCellBlock(pivot_tile.Row, pivot_tile.Column);
                    const auto& [min_row, max_row, min_col, max_col] = helpers::GetMinMaxRowColumnFromCell(cell);

                    for (int row = min_row; row < max_row; ++row) {
                        for (int col = min_col; col < max_col; ++col) {
                            if (pincer_line_alignment ? row == line_idx : col == line_idx) {
                                continue;
                            }

                            if (sudoku_board.IsTileFilled(row, col) || sudoku_board.GetTilePencilMarks(row, col).count() != 7) {
                                continue;
                            }

                            if (sudoku_board.IsTileCandidateUsed(row, col, complementary_number) || sudoku_board.IsTileCandidateUsed(row, col, pincer_number)) {
                                continue;
                            }

                            if (sudoku_board.UpdateCellLinePencilMarks(pincer_tile_1.Cell, pincer_number, (pincer_line_alignment ? row : col), pincer_line_alignment)) {
                                ++count;
                            }
                        }
                    }
                };

                auto find_second_pincer_in_row = [&count, &sudoku_board](BoardTile& pivot_tile, BoardTile& pincer_tile_1, int pincer_number, int complementary_number) {
                    int pincer_row = pincer_tile_1.Row;
                    int pivot_row = pivot_tile.Row;
                    int starting_col = ((pivot_tile.Column / 3) * 3) + 3;
                    if (starting_col == 9) {
                        starting_col = 0;
                    }

                    for (int pincer_col = starting_col; sudoku_board.GetTile(pivot_row, pincer_col).Cell != pivot_tile.Cell; ++pincer_col) {
                        if (sudoku_board.IsTileFilled(pivot_row, pincer_col) || sudoku_board.GetTilePencilMarks(pivot_row, pincer_col).count() != 7) {
                            if (pincer_col == 8) {
                                pincer_col = -1;
                            }
                            continue;
                        }

                        if (sudoku_board.IsTileCandidateUsed(pivot_row, pincer_col, complementary_number) || sudoku_board.IsTileCandidateUsed(pivot_row, pincer_col, pincer_number)) {
                            if (pincer_col == 8) {
                                pincer_col = -1;
                            }
                            continue;
                        }

                        if (!sudoku_board.IsTileFilled(pincer_row, pincer_col) && !sudoku_board.IsTileCandidateUsed(pincer_row, pincer_col, pincer_number)) {
                            sudoku_board.BoardTiles[pincer_row][pincer_col].Pencilmarks.set(pincer_number);
                            ++count;
                        }

                        if (pincer_col == 8) {
                            pincer_col = -1;
                        }
                    }
                };

                auto find_second_pincer_in_col = [&count, &sudoku_board](BoardTile& pivot_tile, BoardTile& pincer_tile_1, int pincer_number, int complementary_number) {
                    int pincer_col = pincer_tile_1.Column;
                    int pivot_col = pivot_tile.Column;
                    int starting_row = ((pivot_tile.Row / 3) * 3) + 3;
                    if (starting_row == 9) {
                        starting_row = 0;
                    }

                    for (int pincer_row = starting_row; sudoku_board.GetTile(pincer_row, pivot_col).Cell != pivot_tile.Cell; ++pincer_row) {
                        if (sudoku_board.GetTile(pincer_row, pivot_col).IsTileFilled() || sudoku_board.GetTilePencilMarks(pincer_row, pivot_col).count() != 7) {
                            if (pincer_row == 8) {
                                pincer_row = -1;
                            }
                            continue;
                        }

                        if (sudoku_board.IsTileCandidateUsed(pincer_row, pivot_col, complementary_number) || sudoku_board.IsTileCandidateUsed(pincer_row, pivot_col, pincer_number)) {
                            if (pincer_row == 8) {
                                pincer_row = -1;
                            }
                            continue;
                        }

                        if (!sudoku_board.IsTileFilled(pincer_row, pincer_col) && !sudoku_board.IsTileCandidateUsed(pincer_row, pincer_col, pincer_number)) {
                            sudoku_board.GetTile(pincer_row, pincer_col).Pencilmarks.set(pincer_number);
                            ++count;
                        }

                        if (pincer_row == 8) {
                            pincer_row = -1;
                        }
                    }
                };

                find_second_pincer_in_cell(*pivot_tile_1, pivot_tile_2, pincer_number, complementary_pivot_number);
                find_second_pincer_in_cell(pivot_tile_2, *pivot_tile_1, complementary_pivot_number, pincer_number);

                // This is a flag to not repeat the same operation after the first one (you usually find y-wings through rows then columns)
                if (!stop_finding_in_lines) {
                    pincer_line_alignment ? find_second_pincer_in_col(*pivot_tile_1, pivot_tile_2, pincer_number, complementary_pivot_number) : find_second_pincer_in_row(*pivot_tile_1, pivot_tile_2, pincer_number, complementary_pivot_number);
                    pincer_line_alignment ? find_second_pincer_in_col(pivot_tile_2, *pivot_tile_1, complementary_pivot_number, pincer_number) : find_second_pincer_in_row(pivot_tile_2, *pivot_tile_1, complementary_pivot_number, pincer_number);
                }
            }
        };

        find_y_wing_lambda(RowOrColumn_Row, pivot_tile_1->Row, true);
        find_y_wing_lambda(RowOrColumn_Column, pivot_tile_1->Column, false);
    }

    return count;
}
//
std::tuple<size_t, size_t, size_t> FindFishes(GameBoard& sudoku_board) noexcept
{
    size_t xwing_count     = 0;
    size_t swordfish_count = 0;
    size_t jellyfish_count = 0;

    // Initialized before the loop so no multiple instances of recreating the vector over and over. 
    // Also preserves the capacity so resizing will be minimal or none at all.
    std::vector<BoardTile*> first_fish_tiles; first_fish_tiles.reserve(4);  // This contains the first swordfish tiles. The basis of finding other fishes
    std::vector<BoardTile*> other_fish_tiles; other_fish_tiles.reserve(4);  // This contains the other swordfish tiles
    std::vector<BoardTile*> final_fish_tiles; final_fish_tiles.reserve(12); // This contains the combination of first and other swordfish tiles

    auto final_swordfish_step = [&xwing_count, &swordfish_count, &jellyfish_count, &final_fish_tiles, &sudoku_board](int row_or_column, int fish_size, int bit_num) {
        // A flag for a stray line for early exit
        bool stray_line = false;
        std::vector<int> aligned_lines;
        if (row_or_column == RowOrColumn_Row) {
            // If the fishes are in row then the fishes should have aligned in columns
            for (int line_idx = 0; line_idx < 9; ++line_idx) {
                const size_t line_count = std::count_if(final_fish_tiles.begin(), final_fish_tiles.end(), [&](const BoardTile* obj) { return obj->Column == line_idx; });
                if (line_count == 1) {
                    stray_line = true;
                    break;
                }
                if (line_count >= 2) {
                    aligned_lines.push_back(line_idx);
                }
            }
        }
        else {
            // If the fishes are in column then the fishes should have aligned in rows
            for (int line_idx = 0; line_idx < 9; ++line_idx) {
                const size_t line_count = std::count_if(final_fish_tiles.begin(), final_fish_tiles.end(), [&](const BoardTile* obj) { return obj->Row == line_idx; });
                if (line_count == 1) {
                    stray_line = true;
                    break;
                }
                if (line_count >= 2) {
                    aligned_lines.push_back(line_idx);
                }
            }
        }

        if (stray_line || aligned_lines.size() != fish_size) {
            return;
        }

        bool success = false;
        for (auto& line_idx : aligned_lines) {
            success = (row_or_column ? sudoku_board.UpdateColumnPencilMarks(line_idx, bit_num, final_fish_tiles) : sudoku_board.UpdateRowPencilMarks(line_idx, bit_num, final_fish_tiles)) || success;
        }
        if (success) {
            switch (fish_size)
            {
            case 2: ++xwing_count;     break;
            case 3: ++swordfish_count; break;
            case 4: ++jellyfish_count; break;
            default: break;
            }
        }
    };

    auto find_fishes_in_row = [&](int fish_size) {
        for (int row = 0; row < 6; ++row) {
            for (int bit_num = 0; bit_num < 9; ++bit_num) {
                first_fish_tiles.clear();
                for (int col = 0; col < 9; ++col) {
                    if (sudoku_board.IsTileFilled(row, col) || sudoku_board.IsTileCandidateUsed(row, col, bit_num)) {
                        continue;
                    }

                    first_fish_tiles.push_back(&sudoku_board.GetTile(row, col));
                }

                if (first_fish_tiles.size() > fish_size || first_fish_tiles.size() < 2) {
                    continue;
                }
                if (fish_size == 2 && first_fish_tiles[0]->Cell == first_fish_tiles[1]->Cell) {
                    continue;
                }

                auto find_other_fish_tiles = [&sudoku_board, &fish_size, &final_fish_tiles, &other_fish_tiles, &bit_num](int& current_row, auto& self_function) {
                    current_row++;
                    if (current_row == 9) {
                        return false;
                    }

                    other_fish_tiles.clear();
                    for (int col = 0; col < 9; ++col) {
                        if (!sudoku_board.IsTileFilled(current_row, col) && !sudoku_board.IsTileCandidateUsed(current_row, col, bit_num)) {
                            other_fish_tiles.push_back(&sudoku_board.GetTile(current_row, col));
                        }
                    }

                    if (other_fish_tiles.size() > fish_size || other_fish_tiles.size() < 2) {
                        return self_function(current_row, self_function);
                    }

                    final_fish_tiles.insert(final_fish_tiles.end(), other_fish_tiles.begin(), other_fish_tiles.end());
                    return true;
                };

                for (int starting_row_1 = row; starting_row_1 < 9; ) {
                    final_fish_tiles = first_fish_tiles;
                    if (!find_other_fish_tiles(starting_row_1, find_other_fish_tiles)) {
                        break;
                    }

                    if (fish_size == 2) {
                        final_swordfish_step(RowOrColumn_Row, fish_size, bit_num);
                        continue;
                    }

                    for (int starting_row_2 = starting_row_1; starting_row_2 < 9; ) {
                        if (!find_other_fish_tiles(starting_row_2, find_other_fish_tiles)) {
                            break;
                        }

                        if (fish_size == 3) {
                            final_swordfish_step(RowOrColumn_Row, fish_size, bit_num);
                            continue;
                        }

                        for (int starting_row_3 = starting_row_2; starting_row_3 < 9; ) {
                            if (!find_other_fish_tiles(starting_row_3, find_other_fish_tiles)) {
                                break;
                            }

                            final_swordfish_step(RowOrColumn_Row, fish_size, bit_num);
                        }
                    }
                }
            }
        }
    };

    auto find_fishes_in_column = [&](int fish_size) {
        for (int col = 0; col < 6; ++col) {
            for (int bit_num = 0; bit_num < 9; ++bit_num) {
                first_fish_tiles.clear();
                for (int row = 0; row < 9; ++row) {
                    if (sudoku_board.IsTileFilled(row, col) || sudoku_board.IsTileCandidateUsed(row, col, bit_num)) {
                        continue;
                    }

                    first_fish_tiles.push_back(&sudoku_board.GetTile(row, col));
                }

                if (first_fish_tiles.size() > fish_size || first_fish_tiles.size() < 2) {
                    continue;
                }
                if (fish_size == 2 && first_fish_tiles[0]->Cell == first_fish_tiles[1]->Cell) {
                    continue;
                }

                auto find_other_fish_tiles = [&sudoku_board, &fish_size, &final_fish_tiles, &other_fish_tiles, &bit_num](int& current_col, auto& self_function) {
                    current_col++;
                    if (current_col == 9) {
                        return false;
                    }

                    other_fish_tiles.clear();
                    for (int row = 0; row < 9; ++row) {
                        if (!sudoku_board.IsTileFilled(row, current_col) && !sudoku_board.IsTileCandidateUsed(row, current_col, bit_num)) {
                            other_fish_tiles.push_back(&sudoku_board.GetTile(row, current_col));
                        }
                    }

                    if (other_fish_tiles.size() > fish_size || other_fish_tiles.size() < 2) {
                        return self_function(current_col, self_function);
                    }

                    final_fish_tiles.insert(final_fish_tiles.end(), other_fish_tiles.begin(), other_fish_tiles.end());
                    return true;
                };

                for (int starting_col_1 = col; starting_col_1 < 9; ) {
                    final_fish_tiles = first_fish_tiles;
                    if (!find_other_fish_tiles(starting_col_1, find_other_fish_tiles)) {
                        break;
                    }

                    if (fish_size == 2) {
                        final_swordfish_step(RowOrColumn_Column, fish_size, bit_num);
                        continue;
                    }

                    for (int starting_col_2 = starting_col_1; starting_col_2 < 9; ) {
                        if (!find_other_fish_tiles(starting_col_2, find_other_fish_tiles)) {
                            break;
                        }

                        if (fish_size == 3) {
                            final_swordfish_step(RowOrColumn_Column, fish_size, bit_num);
                            continue;
                        }

                        for (int starting_col_3 = starting_col_2; starting_col_3 < 9; ) {
                            if (!find_other_fish_tiles(starting_col_3, find_other_fish_tiles)) {
                                break;
                            }

                            final_swordfish_step(RowOrColumn_Column, fish_size, bit_num);
                        }
                    }
                }
            }
        }
    };

    // X-Wing Technique
    find_fishes_in_row(2);
    find_fishes_in_column(2);

    // Swordfish technique
    find_fishes_in_row(3);
    find_fishes_in_column(3);

    // Jellyfish technique
    find_fishes_in_row(4);
    find_fishes_in_column(4);

    return { xwing_count, swordfish_count, jellyfish_count };
}

}



