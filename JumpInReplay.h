#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#pragma comment( lib, "pluginsdk.lib")

#include <vector>


constexpr float MAX_DODGE_TIME = 1.2f;

struct CarPosition
{
	Vector location;
	Vector velocitiy;
	Rotator rotation;
	Vector angVelocity;
	float steer;
	float throttle;
	bool handbrake;
	float boostAmount;
	bool isBoosting;
	//float lastJumped;
	//bool hasDodge;
	int ballTouches;
};

struct BallPosition
{
	Vector location;
	Vector velocitiy;
	Rotator rotation;
	Vector angVelocity;
};

struct GameInfo
{
	int TimeRemaining;
	int OrangeScore;
	int BlueScore;
	//bool IsInOvertime;
	int Countdowntime;
	//bool IsInCountdown;
};

class JumpInReplay: public BakkesMod::Plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();

	void CvarRegister();

	//cvar commands
	void Test(std::string oldValue, CVarWrapper cvar);
	void SaveReplay(std::string oldValue, CVarWrapper cvar);
	void StartPrivateMatch(std::string oldValue, CVarWrapper cvar);
	void SpawnBots(std::string oldValue, CVarWrapper cvar);
	void ControllGame(std::string oldValue, CVarWrapper cvar);
	void Skip(std::string oldValue, CVarWrapper cvar);
	void SwitchCars(std::string oldValue, CVarWrapper cvar);
	void SwitchBack(std::string oldValue, CVarWrapper cvar);
	void JumpIn(std::string oldValue, CVarWrapper cvar);
	void Convert(std::string oldValue, CVarWrapper cvar);
	void OpenReplay(std::string oldValue, CVarWrapper cvar);
	void Bindings(std::string oldValue, CVarWrapper cvar);
	void AutoConvertCvar(std::string oldValue, CVarWrapper cvar);

	//cvar variablas

	//Function
	void AutoConvert();
	void SaveCar(int i);
	void SaveCar0();
	void SaveBall();
	void SaveGameInfo();
	void SpawnOneBot(int i);
	void SetTeams();
	void SetCar(int i, int it);
	void SetCar0(int i);
	void SetBall();
	void SetGameInfo();

	//Event Hooks
	void TestHook(std::string eventName);
	void SaveGameState(std::string eventName);
	void ControllGamePerTick(std::string eventName);
	void JoinedFreeplay(std::string eventName);
	void IsInReplay(std::string eventName);

	//One Time Saves
	bool hasReplaySaved = false;
	void OneTimeSaves();
	int LobbySize = 0;
	float ReplayHZ = 30;
	int ReplayFactor = 4;
	std::vector<std::string> playerNames;
	std::vector<int> CarLayouts;
	int Gamemode = 0;
	std::string Arena;
	std::vector<unsigned long> StartCars;
	std::vector<int> StartTeams;
	//Car designs
	std::vector<LinearColor> primeColor;
	std::vector<LinearColor> secondColor;
	int hasColorsNormalized;

	//replay variables
	int Frame = 0;
	int ReplaySize = 0;
	int SavedFrames = 0;
	int TempBlueScore = 0;
	int TempOrangeScore = 0;

	int Tick = 8;
	int FrameTick = 0;
	int CarAmount = 0;
	int CountdownTime = 0;
	int TimeRemaining = 0;
	int OrangeScore = 0;
	int BlueScore = 0;
	int Score = 0;
	bool isInOvertime = false;
	bool setBally = true;

	//saves per frame
	std::vector<CarPosition> CarPositionsPerFrame;
	std::vector<BallPosition> BallPositionPerFrame;
	std::vector<GameInfo> GameInfoPerFrame;

	//replay saves
	std::vector<int> BlueScoredFrame;
	std::vector<int> OrangeScoredFrame;

	//switch car variables
	int carit = 0;

	//JumpIn
	int BallHits = 0;
	int JumpInTick = 0;


	//Overlay
	void DrawHud(CanvasWrapper canvas);
	void DrawTime(CanvasWrapper canvas);
	void DrawPause(CanvasWrapper canvas);
	void DrawTimeline(CanvasWrapper canvas);
	void DrawTimelineSpent(CanvasWrapper canvas);
	void DrawPlayer(CanvasWrapper canvas);
	void DrawJumpIn(CanvasWrapper canvas);
	void DrawOrangeScored(CanvasWrapper canvas);
	void DrawBlueScored(CanvasWrapper canvas);

	//Overlay variables
	int MinutesAvailable = 0;
	int SecondsAvailable = 0;
	int MinutesDone = 0;
	int SecondsDone = 0;

private:

	void Log(std::string msg);

};

