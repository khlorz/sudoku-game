#pragma once

#include <vector>
#include <array>
#include <bitset>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <random>
#include <chrono>
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/bitset.hpp"
#include "boost/serialization/split_member.hpp"

using SolveMethod         = int;
using SolveStartWith      = int;
using SudokuDifficulty    = int;
using UsedSudokuTechnique = int;

enum RowOrColumn_
{
    RowOrColumn_Column = 0,
    RowOrColumn_Row    = 1
};

enum SudokuDifficulty_
{
    SudokuDifficulty_Random     = 0,
    SudokuDifficulty_Easy       = 1,
    SudokuDifficulty_Normal     = 2,
    SudokuDifficulty_Insane     = 3,
    SudokuDifficulty_Diabolical = 4
};

enum UsedSudokuTechnique_
{
    UsedSudokuTechnique_None           = 0,
    UsedSudokuTechnique_CandidateLines = 1 << 0,
    UsedSudokuTechnique_Intersections  = 1 << 1,
    UsedSudokuTechnique_NakedPair      = 1 << 2,
    UsedSudokuTechnique_NakedTriple    = 1 << 3,
    UsedSudokuTechnique_NakedQuad      = 1 << 4,
    UsedSudokuTechnique_HiddenPair     = 1 << 5,
    UsedSudokuTechnique_HiddenTriple   = 1 << 6,
    UsedSudokuTechnique_HiddenQuad     = 1 << 7,
    UsedSudokuTechnique_XWing          = 1 << 8,
    UsedSudokuTechnique_YWing          = 1 << 9,
    UsedSudokuTechnique_SwordFish      = 1 << 10,
    UsedSudokuTechnique_JellyFish      = 1 << 11
};

enum SolveMethod_
{
    SolveMethod_Humanely   = 1,    // Solve the sudoku like a human would with known sudoku techniques
    SolveMethod_BruteForce = 2,    // Typical sudoku solver method with simple backtracking method
    SolveMethod_MRV        = 3     // Sudoku solver method that uses minimum remaining values for faster solving
};

// Sudoku namespace
namespace sdq
{

class BoardOccurences
{
private:
    std::array<std::bitset<9>, 9> RowOccurences;
    std::array<std::bitset<9>, 9> ColOccurences;
    std::array<std::bitset<9>, 9> CellOccurences;

public:
    BoardOccurences();

    //Query
    constexpr bool IsEmpty() noexcept;

    // Getters
    std::bitset<9>& GetRowOccurences(int row)  noexcept;
    std::bitset<9>& GetColumnOccurences(int col)  noexcept;
    std::bitset<9>& GetCellOccurences(int cell) noexcept;
    std::bitset<9>& GetCellOccurences(int row, int col) noexcept;
    std::bitset<9>  GetTileOccurences(int row, int col) const noexcept;

    // Setters
    void ResetAll();
    void SetCellNumber(int row, int col, int number) noexcept;
    void ResetCellNumber(int row, int col, int number) noexcept;

};

// Structure holds the important parameters of a sudoku tile
struct BoardTile
{
private:
    std::bitset<9>* RowOccurence = nullptr, * ColOccurence = nullptr, * CellOccurence = nullptr;

public:
    int             TileNumber;
    int             Row, Column, Cell;
    std::bitset<9>  Pencilmarks;

    BoardTile() : TileNumber('0'), Row(-1), Column(-1), Cell(-1), Pencilmarks(0) {}
    BoardTile(int tile_number, int row, int col, BoardOccurences& board_occurences) noexcept;

    void            Initialize(int tile_number, int row, int col, BoardOccurences& board_occurences) noexcept;
    void            Clear() noexcept;
    void            SetTileNumber(int bit_number) noexcept;
    void            ResetTileNumber() noexcept;
    void            InitializeTileOccurences(BoardOccurences& board_occurences) noexcept;
    void            RemovePencilmarks();
    void            ReapplyPencilmarks();
    void            ResetPencilmarks();
    bool            IsTileFilled() const noexcept;
    std::bitset<9>  GetTileOccurences() noexcept;

    bool operator == (const BoardTile& other) const noexcept;

};

// Contains the sudoku board object
struct GameBoard
{
    bool                                    BoardInitialized;
    std::array<std::array<BoardTile, 9>, 9> BoardTiles;
    BoardOccurences                         BoardOccurences;
    std::vector<BoardTile*>                 PuzzleTiles;

    GameBoard();
    GameBoard(const GameBoard& other) noexcept;

    // Operators
    GameBoard& operator = (const GameBoard& other) noexcept;
    bool       operator == (const GameBoard& other) const noexcept;

    bool       CreateSudokuBoard(const std::array<std::array<int, 9>, 9>& sudoku_board, bool create_puzzletiles_vec = true) noexcept;
    void       UpdateBoardOccurences() noexcept;
    void       UpdateBoardOccurences(int row, int column) noexcept;
    void       ClearSudokuBoard() noexcept;
    bool       IsBoardCompleted() const noexcept;
    void       CreatePuzzleTiles() noexcept;
    BoardTile* FindNextEmptyPosition(int row, int col) noexcept;
    BoardTile* FindNextEmptyPosition() noexcept;
    BoardTile* FindLowestMRV() noexcept;

    bool             IsTileFilled(int row, int column) noexcept;
    BoardTile&       GetTile(int row, int column) noexcept;
    const BoardTile& GetTile(int row, int column) const noexcept;
    std::bitset<9>&  GetTilePencilMarks(int row, int column) noexcept;
    bool             IsTileCandidateUsed(int row, int column, int bit_number) noexcept;
    bool             IsCandidatePresentInTheSameCell(int cell, int bit_number, const std::vector<BoardTile*> exempted_tiles) noexcept;
    bool             IsCandidatePresentInTheSameLine(int line_index, int bit_number, int row_or_column, const std::vector<BoardTile*> exempted_tiles) noexcept;

    void  UpdateRemovePencilMarks() noexcept;
    void  UpdateReapplyPencilMarks() noexcept;
    void  ResetAllPencilMarks() noexcept;
    bool  UpdateRowPencilMarks(int row, int bit_number, const std::vector<int>& exempted_cells) noexcept;
    bool  UpdateRowPencilMarks(int row, int bit_number, const std::vector<BoardTile*>& exempted_tiles) noexcept;
    bool  UpdateColumnPencilMarks(int col, int bit_number, const std::vector<int>& exempted_cells) noexcept;
    bool  UpdateColumnPencilMarks(int col, int bit_number, const std::vector<BoardTile*>& exempted_tiles) noexcept;
    bool  UpdateCellPencilMarks(int cell, int bit_number, const std::vector<BoardTile*>& exempted_tiles) noexcept;
    bool  UpdateCellPencilMarks(int cell, int bit_number, int exempted_line_idx, int row_or_column) noexcept;
    bool  UpdateCellLinePencilMarks(int cell, int bit_number, int line_idx, int row_or_column) noexcept;
    bool  UpdateTilePencilMarks(const std::vector<BoardTile*>& sudoku_tiles, const std::vector<int> exempted_numbers) noexcept;

private:
    bool CreateBoardOccurences(const std::array<std::array<int, 9>, 9>& board) noexcept;
};

class TurnLog
{
public:
    struct TurnTile
    {
        int Row;
        int Column;
        int PreviousNumber;
        int NextNumber;
        std::bitset<9> PreviousPencilmark;
        std::bitset<9> NextPencilmark;

        TurnTile() noexcept 
            : Row(0), Column(0), PreviousNumber(0), NextNumber(0), PreviousPencilmark(0), NextPencilmark(0) {};
        TurnTile(int _row, int _col, int prev_num, int next_num, const std::bitset<9>& prev_pm, const std::bitset<9>& next_pm) noexcept 
            : Row(_row), Column(_col), PreviousNumber(prev_num), NextNumber(next_num), PreviousPencilmark(prev_pm), NextPencilmark(next_pm) {}
    };
private:
    std::vector<TurnTile> TilesUsed;
    size_t UndoPosition;
public:
    TurnLog() noexcept;
    void Add(int _row, int _col, int prev_num, int next_num, const std::bitset<9>& prev_pm = 0, const std::bitset<9>& next_pm = 0) noexcept;
    void Undo() noexcept;
    void Redo() noexcept;
    void Reset() noexcept;
    bool CanUndo() const noexcept;
    bool CanRedo() const noexcept;
    const TurnTile* GetUndoTile() const noexcept;
    const TurnTile* GetRedoTile() const noexcept;
};

class Time
{
private:
    std::chrono::steady_clock::time_point              StartingTime;
    std::chrono::steady_clock::time_point              EndTime;
    std::vector<std::chrono::steady_clock::time_point> PausesPoint;

public:
    Time() = default;
    void Reset();
    void Start();
    void End();
    void Pause();
    std::optional<std::tm> GetTimeDuration();
};

// Class for maintaining and holding sudoku game instance
class Instance
{
private:
    SudokuDifficulty   GameDifficulty;    // Stores the current game difficulty
    SudokuDifficulty   RandomDifficulty;  // Store the actual difficulty if the game difficulty is random or custom
    size_t             MaxRemovedTiles;   // Max removed tiles for certain difficulties. The lower it is, the easier the difficulty could be
    std::mt19937_64    GameRNG;           // Sudoku's Random Number Generator for generating the puzzle
    GameBoard          SolutionBoard;     // Stores the solution of the sudoku board
    GameBoard          PuzzleBoard;       // Stores the puzzle of the sudoku board
    TurnLog            GameTurnLogs;

public:
    Instance();

    // Initialized the game with a pre-made sudoku board
    bool CreateSudoku(const std::array<std::array<int, 9>, 9>& board) noexcept;
    // Initialized the game with a random sudoku board
    bool CreateSudoku(SudokuDifficulty game_difficulty) noexcept;
    // Initialize the game with a save progress
    bool LoadSudokuSave(const char* filepath) noexcept;
    bool SaveCurrentProgress(const char* filepath) noexcept;
    // Checks if the current puzzle is already finished
    bool CheckPuzzleState() const noexcept;

    // Getters
    const GameBoard*        GetPuzzleBoard() const noexcept;
    const GameBoard*        GetSolutionBoard() const noexcept;
    SudokuDifficulty        GetBoardDifficulty() const noexcept;
    const TurnLog*          GetTurnLogs() const noexcept;

    // Setters
    bool SetTile(int row, int col, int number) noexcept;
    bool ResetTile(int row, int col) noexcept;
    void RemovePencilmark(int row, int col, int number) noexcept;
    void AddPencilmark(int row, int col, int number) noexcept;
    void ResetAllPencilmarks() noexcept;
    void UpdateAllPencilmarks() noexcept;
    bool IsValidTile(int row, int col) noexcept;
    void UndoTurn() noexcept;
    void RedoTurn() noexcept;


private:
    void ClearAllBoards() noexcept;
    bool CreateCompleteBoard() noexcept;
    bool GeneratePuzzle() noexcept;
    void InitializeGameParameters(SudokuDifficulty game_difficulty) noexcept;

    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive& archive, const uint32_t version) const;
    template<class Archive>
    void load(Archive& archive, const uint32_t version);
    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

namespace helpers
{

constexpr int GetNextRow(int row, int col) noexcept;
constexpr int GetNextCol(int col) noexcept;
constexpr int GetCellBlock(int row, int col) noexcept;
constexpr std::tuple<int, int, int, int> GetMinMaxRowColumnFromCell(int const cell) noexcept;

}

// sudoku solvers and helpers
namespace solvers
{

//----------------------------------------------------------------------------------------------------------------------------------------------
// Sudoku Solvers Helper Functions
//----------------------------------------------------------------------------------------------------------------------------------------------

bool
Solve(GameBoard& sudoku_board, SolveMethod method = SolveMethod_BruteForce) noexcept;

//----------------------------------------------------------------------------------------------------------------------------------------------
// Sudoku Solving Functions
//----------------------------------------------------------------------------------------------------------------------------------------------

// A faster sudoku solver function using Minimum Remaining Values. Used for solving external sudoku boards
bool
SolveMRV(GameBoard& puzzle_board) noexcept;
bool
SolveMRVEX(GameBoard& sudoku_board) noexcept;
// This sudoku solver function is better used for difficulty finder due to its brute force method
bool
SolveBruteForce(GameBoard& puzzle_board) noexcept;
bool
SolveBruteForceEX(GameBoard& sudoku_board) noexcept;
// 
bool
SolveHumanely(GameBoard& sudoku_board, size_t* difficulty_score = nullptr) noexcept;
bool
SolveHumanelyEX(GameBoard& sudoku_board, size_t& difficulty_score) noexcept;

}

// sudoku utilities for helping the solvers and other things
namespace utils
{

// Print the pencilmarks of the board on console. Debug uses for checking the solvers credibility
void
PrintPencilMarks(const GameBoard& sudoku_board) noexcept;
// Print the sudoku board on console
void
PrintBoard(const sdq::GameBoard& board) noexcept;
// A sudoku solver function but its job is to fill the remaining blanks to create a sudoku board
bool
FillSudoku(GameBoard& sudoku_board, const std::array<int, 9>& random_numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9}) noexcept;
// Checks if the board has a unique solution
bool
IsUniqueBoard(GameBoard& sudoku_board) noexcept;
// Check for the difficulty of the sudoku_board
SudokuDifficulty
CheckPuzzleDifficulty(const GameBoard& sudoku_board) noexcept;
SudokuDifficulty 
CheckPuzzleDifficulty(GameBoard& sudoku_board) noexcept;
//
std::optional<std::array<std::array<int, 9>, 9>> 
OpenSudokuFile(const char* filename) noexcept;
bool
CreateSudokuFile(const GameBoard& sudoku_board, const char* filepath) noexcept;

}

// sudoku techniques for solving puzzles
namespace techs
{

/* @returns The number of times single position is used */
size_t
FindSinglePosition(GameBoard& sudoku_board) noexcept;
/* @returns The number of times single candidates is used */
size_t 
FindSingleCandidates(GameBoard& sudoku_board) noexcept;
/* @returns The number of times candidate lines is used */
size_t
FindCandidateLines(GameBoard& sudoku_board) noexcept;
/* @returns The number of times intersections is used */
size_t
FindIntersections(GameBoard& sudoku_board) noexcept;
/* 
*  @returns 
*  first tuple  = hidden pair count;
*  second tuple = hidden triple count;
*  third tuple  = hidden quad count;
*/
std::tuple<size_t, size_t, size_t>
FindNakedTuples(GameBoard& sudoku_board) noexcept;
/* 
*  @returns 
*  first tuple  = hidden pair count; 
*  second tuple = hidden triple count; 
*  third tuple  = hidden quad count; 
*/
std::tuple<size_t, size_t, size_t>
FindHiddenTuples(GameBoard& sudoku_board) noexcept;
/* @returns The number of times x-wing is used */
size_t
FindYWings(GameBoard& sudoku_board) noexcept;
/* @returns 
*  first tuple = xwing count;
*  second tuple = swordfish count;
*  third tuple = jellyfish count
*/
std::tuple<size_t, size_t, size_t>
FindFishes(GameBoard& sudoku_board) noexcept;

}

}