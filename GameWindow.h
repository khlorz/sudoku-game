#pragma once

#include "imgui.h"
#include "sdq.h"
#include "ImFunks.h"
#include <future>
#include <thread>
#include <filesystem>
#include <ctime>

enum TileState_
{
	TileState_Normal               = 1,
	TileState_Solution             = 2,
	TileState_Pencilmark           = 4,
	TileState_Blank                = 8
};
using TileState = int;

struct SudokuTile
{
private:
	int  TileID;                        // Unique ID / number of the object
	bool Initialized;                   // Initialized flag so the unique id number can't be changed
	char ButtonLabel[32];
	char ContextPopUpLabel[32];
	char InputIntLabel[32];

	int                   InputTileNumber;
	const int*            TileNumber;
	const int*            SolutionNumber;
	const std::bitset<9>* Pencilmark;

	bool ShowAsPencilmark;
	bool ShowAsSolution;                // A bool to show the solution number instead of the current number
	bool PuzzleTile;                    // The tile that can't be changed or is the puzzle number
	bool ErrorTile;                     // A bool to show if the tile is an error (has duplicate number in the row or column or cell)

public:
	static constexpr size_t CharBufferSize = 32;
	static constexpr ImVec2 ButtonSize = ImVec2(48.50f, 48.50f);

public:
	SudokuTile();
	SudokuTile(const sdq::BoardTile& tile, const sdq::BoardTile& solution_tile);
	// Initializes the tile id and tile number
	// Should only be used once!
	bool InitializeTile(const sdq::BoardTile& puzzle_tile, const sdq::BoardTile& solution_tile);
	// Sets the number of the tile
	void UpdateTileNumber(TileState tile_state);
	// Sets the tile as a puzzle number
	void SetTilePuzzleNumber(uint16_t t_number, uint16_t solution_number, bool is_puzzle);
	//
	bool IsPuzzleTile();
	//
	bool IsTileFilled();
	//
	void RecheckError(sdq::Instance& sudoku_context, uint16_t row, uint16_t col);
	// Renders the tile button and it's popup context
	// Returns false if the input is not valid, otherwise true
	bool RenderButton(sdq::Instance& sudoku_context, uint16_t row, uint16_t col, bool error_override);
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
	bool             GamePaused;
	bool             WindowClose;
	bool             ShowSolution;
	bool             ShowError;
	bool             CheckGameState;
	bool             OpenGameEndWindow;
	bool             OpenLoadSudokuWindow;
	bool             OpenLoadSaveFileWindow;
	bool             ShowPencilmarks;
	bool             ShowLoadingScreen;
	bool             StartLoadingScreen;
	bool             SudokuFileSaved;
	bool             LoadAFile;
	TimeObj          TimeElapsed;
	TimeObj          ShowSolutionTotalTime;
	sdq::Instance    SudokuContext;
	SudokuTiles<9>   SudokuGameTiles;
	SudokuDifficulty GameDifficulty;
	std::string      CurrentlyOpenFile;

	mutable std::mutex     NewGameMutex;
	std::future<bool>      NewGameFuture;
	ImFunks::LoadingScreen NewGameLoading;
	std::optional<bool>    NewGameResult;
public:
	GameWindow();

	void RenderWindow();
	bool IsWindowClosed();

private:
	// Windows
	void GameEndWindow();
	bool GameMainWindow();
	void LoadSudokuFileWindow();
	void LoadSaveFileWindow();
	void GameOptions();
	void MainMenuBar();

	// Process Functions
	bool CreateNewGame(const std::string& filepath);
	bool CreateNewGame(SudokuDifficulty difficulty);
	bool LoadSaveFile(const std::string& filepath);
	bool SaveProgress(const std::string& filepath);
	void StopOngoingGame();
	void SetSudokuTilesForNewGame();
	void RenderSudokuBoard();
	void SetShowSolution();
	void Update();
	void RecheckTiles();
	void SetSudokuTileFromSaveFile();
	void PencilmarkOptions();
	void UndoRedoOptions();
	void TimeOptions();
	void ErrorAndSolutionOptions();
	void SaveProgressOption();
	void SavePuzzleOption();
	void NewGameOption();

	// Loading Screen Functions
	void RenderNewGameLoadingScreen();
	void StartNewGameLoadingScreen();
	void CheckNewGameProgress();
};

