#pragma once

#include <vector>
#include <array>
#include <bitset>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <random>
#include <chrono>

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
    bool            PuzzleTile;
    int             Row, Column, Cell;
    std::bitset<9>  Pencilmarks;        

    BoardTile() : TileNumber('0'), PuzzleTile(false), Row(-1), Column(-1), Cell(-1), Pencilmarks(0) {}
    BoardTile(std::array<std::array<int, 9>, 9>& board, BoardOccurences& board_occurences, int row, int col, int cell) noexcept;

    void            CreateSudokuTile(const std::array<std::array<int, 9>, 9>& board, BoardOccurences& board_occurences, int row, int col, int cell) noexcept;
    void            ClearTile() noexcept;
    void            SetTileNumber(int bit_number) noexcept;
    void            ResetTileNumber() noexcept;
    void            InitializeTileOccurences(BoardOccurences& board_occurences) noexcept;
    void            UpdatePencilMarks();
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

    bool       CreateSudokuBoard(const std::array<std::array<int, 9>, 9>& sudoku_board) noexcept;
    void       UpdateBoardOccurences() noexcept;
    void       UpdateBoardOccurences(int row, int column) noexcept;
    void       ClearSudokuBoard() noexcept;
    bool       IsBoardCompleted() const noexcept;
    bool       CreatePuzzleTiles() noexcept;
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

    void  UpdateAllPencilMarks() noexcept;
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

// Class for maintaining and holding sudoku game context
class GameContext
{
private:
    SudokuDifficulty   GameDifficulty;    // Stores the current game difficulty
    SudokuDifficulty   RandomDifficulty;  // Store the actual difficulty if the game difficulty is random or custom
    size_t             MaxRemovedTiles;   // Max removed tiles for certain difficulties. The lower it is, the easier the difficulty could be
    std::mt19937_64    GameRNG;           // Sudoku's Random Number Generator for generating the puzzle
    GameBoard          SolutionBoard;     // Stores the solution of the sudoku board
    GameBoard          PuzzleBoard;       // Stores the puzzle of the sudoku board

public:
    GameContext();

    // Initialized the game with a pre-made sudoku board
    bool CreateSudoku(const std::array<std::array<int, 9>, 9>& board) noexcept;
    // Initialized the game with a random sudoku board
    bool CreateSudoku(SudokuDifficulty game_difficulty) noexcept;
    // Checks if the current puzzle is already finished
    bool CheckPuzzleState() const noexcept;

    // Getters
    const GameBoard*        GetPuzzleBoard() const noexcept;
    const GameBoard*        GetSolutionBoard() const noexcept;
    const SudokuDifficulty& GetBoardDifficulty() const noexcept;

    // Setters
    bool SetTile(int row, int col, int number) noexcept;
    bool IsValidTile(int row, int col) noexcept;

private:
    // Create a sudoku board
    bool CreateSudoku() noexcept;
    // Clear the sudoku board
    void ClearAllBoards() noexcept;
    // Fill the board with random numbers (still abides the sudoku rules)
    bool CreateCompleteBoard() noexcept;
    // Make a puzzle out of the created sudoku board
    bool GeneratePuzzle() noexcept;
    // Initialize the important parameters for creating a sudoku board
    void InitializeGameParameters(SudokuDifficulty game_difficulty) noexcept;

// DEBUG FUNCTIONS
#ifdef _DEBUG
public:
    void FillPuzzleBoard();
#endif

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
SolveHumanelyEX(GameBoard& sudoku_board, size_t& difficulty_score, UsedSudokuTechnique used_techniques = 0) noexcept;
bool
SolveHumanelyLoop(GameBoard& sudoku_board, size_t& difficulty_score) noexcept;

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
CreateSudokuFile(const GameBoard& sudoku_board, const char* directory, const char* filename = nullptr) noexcept;
bool
CreateSudokuFile(const GameContext& sudoku_context, const char* directory, const char* filename = nullptr) noexcept;

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