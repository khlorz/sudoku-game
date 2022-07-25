#pragma once

#include "imgui.h"
#include "sdq.h"

struct SudokuTile
{
private:
	int  TileID;                        // Unique ID / number of the object
	int  TileNumber;                    // The current number of the sudoku tile
	int  SolutionNumber;                // The solution number of the sudoku tile
	bool ShowAsSolution;                // A bool to show the solution number instead of the current number
	bool PuzzleTile;                    // The tile that can't be changed or is the puzzle number
	bool ErrorTile;                     // A bool to show if the tile is an error (has duplicate number in the row or column or cell)
	bool Initialized;                   // Initialized flag so the unique id number can't be changed
	char ButtonLabel[32];
	char ContextPopUpLabel[32];
	char InputIntLabel[32];
	static constexpr size_t CharBufferSize = 32;
	static constexpr ImVec2 ButtonSize = ImVec2(48.50f, 48.50f);

public:
	SudokuTile();
	SudokuTile(int item_num);
	// Initializes the tile id and tile number
	// Should only be used once!
	bool InitializeTile(int item_num);
	// Sets the number of the tile
	void SetTileNumber(uint16_t t_number);
	// Sets the tile as a puzzle number
	void SetTilePuzzleNumber(uint16_t t_number, uint16_t solution_number);
	// Sets the tile to be shown as a solution number
	void SetAsSolutionTile(bool enable);
	// Sets the tile to be shown with pencilmarks
	void SetAsPencilmarkTile(bool enable);
	// Resets the tile
	void ResetTile();
	//
	bool IsPuzzleTile();
	//
	bool IsTileFilled();
	//
	void RecheckError(sdq::GameContext& sudoku_context, uint16_t row, uint16_t col);
	// Renders the tile button and it's popup context
	// Returns false if the input is not valid, otherwise true
	bool RenderButton(sdq::GameContext& sudoku_context, uint16_t row, uint16_t col, bool error_override);
};

template<size_t S>
using SudokuTiles = std::array<std::array<SudokuTile, S>, S>;

struct TimeObj
{
	size_t Days;
	size_t Hours;
	size_t Minutes;
	float  Seconds;

	constexpr TimeObj() noexcept: Days(0), Hours(0), Minutes(0), Seconds(0.0f) {}
	constexpr TimeObj& operator += (float _seconds) noexcept
	{
		this->Seconds += _seconds;
		while (Seconds >= 60.0f) { ++this->Minutes; this->Seconds -= 60.0f; }
		while (Minutes >= 60)    { ++this->Hours;   this->Minutes -= 60; }
		while (Hours   >= 24)    { ++this->Days;    this->Hours   -= 24; }
		return *this;
	}
	constexpr void Clear() noexcept { Days = 0; Hours = 0; Minutes = 0; Seconds = 0.0f; }
};

class GameWindow
{
private:
	bool             Initialized;
	bool             GameStart;
	bool             WindowClose;
	bool             ShouldShowSolution;
	bool             ShouldAlwaysShowError;
	bool             CheckGameState;
	bool             OpenGameEndWindow;
	bool             OpenFileWindow;
	bool             SudokuFileSaved;
	TimeObj          TimeElapsed;
	TimeObj          ShowSolutionTotalTime;
	sdq::GameContext SudokuContext;
	SudokuTiles<9>   SudokuGameTiles;
	SudokuDifficulty GameDifficulty;

public:
	GameWindow();

	void RenderWindow();
	bool IsWindowClosed();

private:
	void FileWindow();
	void RecheckTiles();
	void GameOptions();
	bool CreateNewGame(const char* filepath);
	bool CreateNewGame(SudokuDifficulty difficulty);
	void StopOngoingGame();
	void MainMenuBar();
	void SetSudokuTilesForNewGame();
	void GameEndWindow();
	bool GameMainWindow();
	void RenderSudokuBoard();
	void SetShowSolution();
	void Update();

};