#pragma once

#include "imgui.h"
#include "sdq.h"
#include "ImFunks.h"
#include <future>
#include <thread>
#include <filesystem>
#include <ctime>

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
	void SetTilePuzzleNumber(uint16_t t_number, uint16_t solution_number, bool is_puzzle);
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

struct SaveFile
{
	bool              Exists;
	SudokuDifficulty  Difficulty;
	std::string       Directory;
	std::tm           DateTime;

	SaveFile() : Difficulty(SudokuDifficulty_Random), Exists(false) { Directory.reserve(20); }
	SaveFile(const std::string& filepath) : Directory(filepath), Difficulty(SudokuDifficulty_Random), Exists(false) {}
	SaveFile(const std::filesystem::directory_entry& filedir) : Directory(std::move(filedir.path().string())), Difficulty(SudokuDifficulty_Random), Exists(false) {}
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
	bool             OpenLoadSaveFileWindow;
	bool             SudokuFileSaved;
	bool             LoadAFile;
	TimeObj          TimeElapsed;
	TimeObj          ShowSolutionTotalTime;
	sdq::GameContext SudokuContext;
	SudokuTiles<9>   SudokuGameTiles;
	SudokuDifficulty GameDifficulty;
	std::string      CurrentlyOpenFile;

	mutable std::mutex     NewGameMutex;
	std::future<bool>      NewGameFuture;
	ImFunks::LoadingScreen NewGameLoading;

public:
	GameWindow();

	void RenderWindow();
	bool IsWindowClosed();

private:
	void FileWindow();
	void RecheckTiles();
	void GameOptions();
	bool CreateNewGame(const std::string& filepath);
	bool CreateNewGame(SudokuDifficulty difficulty);
	bool LoadSaveFile(const std::string& filepath);
	bool SaveProgress(const std::string& filepath);
	void StopOngoingGame();
	void MainMenuBar();
	void SetSudokuTilesForNewGame();
	void GameEndWindow();
	bool GameMainWindow();
	void RenderSudokuBoard();
	void SetShowSolution();
	void Update();
	void LoadSaveFileWindow();
	void SetSudokuTileFromSaveFile();
};