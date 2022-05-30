#pragma once

#include <vector>
#include <array>
#include <bitset>
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include <span>

enum SudokuDifficulty_
{
    SudokuDifficulty_Not       = -1,
    SudokuDifficulty_Random    = 0,
    SudokuDifficulty_Easy      = 1,
    SudokuDifficulty_Normal    = 2,
    SudokuDifficulty_Hard      = 3,
    SudokuDifficulty_ChadBrain = 4,
    SudokuDifficulty_Custom    = 5
};

using SudokuDifficulty = int;

class SudokuBoardLogic
{
private:
    std::array<std::bitset<9>, 9> RowNumbers;   // Contains the number of the rows in bitset
    std::array<std::bitset<9>, 9> ColNumbers;   // Contains the number of the columns in bitset
    std::array<std::bitset<9>, 9> CellNumbers;  // Contains the number of the cells in bitset

public:
    SudokuBoardLogic();

    // Getters
    auto GetRowNumber              (uint16_t row) const;
    auto GetColNumber              (uint16_t col) const;
    auto GetCellNumber             (uint16_t cell) const;
    auto GetTileAvailability       (uint16_t row, uint16_t col) const;
    constexpr uint16_t GetCellBlock(uint16_t row, uint16_t col) const noexcept;

    // Setters
    void ResetAll       ();
    void SetCellNumber  (uint16_t row, uint16_t col, uint16_t number) noexcept;
    void ResetCellNumber(uint16_t row, uint16_t col, uint16_t number) noexcept;

};

using SudokuBoard = std::vector<std::vector<char>>;

class Sudoku
{
private:
    SudokuDifficulty GameDifficulty;       // Stores the current game difficulty
    SudokuDifficulty RandomDifficulty;     // Store the actual difficulty if the game difficulty is random or custom
    size_t           MinimumIterations;    // Minimum iterations needed for certain difficulties
    size_t           MaximumIterations;    // Maximum iterations needed for certain difficulties
    size_t           MaxRemovedTiles;      // Max removed tiles for certain difficulties. The lower it is, the easier the difficulty would be
    std::mt19937_64  GameRNG;              // Game's Random Number Generator for generating the puzzle
    SudokuBoard      PuzzleBoard;          // The puzzle board. Should never be modified outside the class except for SetTile function
    SudokuBoardLogic PuzzleLogic;          // The logic of the puzzle board
    SudokuBoard      SolutionBoard;        // The puzzle's actual solution
    SudokuBoardLogic SolutionLogic;        // The actual solutions logic

public:
    Sudoku();

    // Initialized the game with a pre-made sudoku board
    bool InitializeGame(SudokuBoard& board);
    // Initialized the game with a random sudoku board
    bool InitializeGame(SudokuDifficulty game_difficulty);
    // Checks if the current puzzle is already finished
    bool CheckGameState() const;

    // Getters
    SudokuBoard&     GetPuzzleBoard   ();
    SudokuBoard&     GetSolutionBoard ();
    SudokuDifficulty GetGameDifficulty() const;

    // Setters
    bool SetTile            (uint16_t row, uint16_t col, uint16_t number);
    void SetGameDifficulty  (SudokuDifficulty game_difficulty);

// DEBUG FUNCTIONS
#ifdef _DEBUG
    void FillPuzzleBoard();
#endif

private:
    bool             CreateSudoku              ();
    void             InitializeGameParameters  (SudokuDifficulty game_difficulty);
    void             ClearSudokuBoard          ();
    bool             IsValidBoard              ();
    void             FillInitialBoard          ();
    void             GeneratePuzzle            ();
    void             InitializedGameSeed       ();
    SudokuDifficulty CheckPuzzleDifficulty     ();
    constexpr bool   SolveSudoku               (SudokuBoard& board, SudokuBoardLogic& board_logic, uint16_t const row_start, uint16_t const col_start, size_t& iterations) noexcept;
    constexpr bool   SolveSudoku               (SudokuBoard& board, SudokuBoardLogic& board_logic, uint16_t const row_start, uint16_t const col_start) noexcept;
    constexpr int    CountSolutions            (SudokuBoard& board, SudokuBoardLogic& board_logic, uint16_t const row_start, uint16_t const col_start);

};