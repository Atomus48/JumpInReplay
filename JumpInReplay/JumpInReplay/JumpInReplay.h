#pragma once

#pragma comment( lib, "pluginsdk.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
#include "compatibility.h"

#include <vector>
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

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
	bool backCam;
	bool ballCam;
	float camPitch;
	float camYaw;
	//float lastJumped;
	//bool hasDodge;
	int ballTouches;
	int team;
	bool isActive;
	bool isInKickoff;
	//std::string name;
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
	int FrameInReplay;
	//bool IsInCountdown;
};

struct TimeNotConverted
{
	int start;
	int end;
};

class JumpInReplay: public BakkesMod::Plugin::BakkesModPlugin, /*public BakkesMod::Plugin::PluginSettingsWindow,*/ public BakkesMod::Plugin::PluginWindow
{
public:
	virtual void onLoad();
	virtual void onUnload();

	//std::shared_ptr<GameWrapper> _globalGameWrapper = gameWrapper;
	Compatibility compatibility;

	void CvarRegister();

	//cvar commands
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

	//test function
	void Test(std::string oldValue, CVarWrapper cvar);

	//cvar variablas

	//Function
	void AutoConvert();
	void SaveCar(int i);
	void SaveCar0();
	void SaveBall();
	void SaveBall0();
	void SaveGameInfo();
	void SpawnOneBot(int i);
	void SetCar(int i, int it);
	void SetCar0(int i);
	void SetBall();
	void SetGameInfo();

	//Event Hooks
	void SaveGameState(std::string eventName);
	void ControllGamePerTick(std::string eventName);
	void JoinPrivateMatch(std::string eventName);
	void JoinedFreeplay(std::string eventName);
	void IsInReplay(std::string eventName);
	void GoalReplayStart(std::string eventName);
	void GoalReplayEnd(std::string eventName);

	//One Time Saves
	bool hasReplaySaved = false;
	void OneTimeSaves();
	void KeyframeSaves();
	std::vector<int> Keyframes;
	std::vector<int> KeyframesInConvert;
	std::string temppath = gameWrapper->GetBakkesModPath().string() + "/data/jumpInReplay/";
	int LobbySize = 0;
	std::vector<std::string> playerNames;
	std::vector<int> CarLayouts;
	int Gamemode = 0;
	int ReplayFPS = 30;
	std::string GamemodeStr;
	std::string Arena;
	std::string AdditionalMutators;
	//std::vector<unsigned long> StartCars;
	std::vector<int> StartTeams;
	//Car designs
	std::vector<LinearColor> primeColor;
	//std::vector<LinearColor> secondColor;
	//int hasColorsNormalized;

	//postitions text
	std::string positionstxt;
	void SavePositionsTxt();

	//int newCars = 0;

	//replay variables
	int curKeyframe = 0;
	int curKeyframeInConvert = 0;
	std::vector<TimeNotConverted> timesNotConverted;
	//int Frame = 0;
	int ReplayTick = 0;
	int ReplaySize = 0;
	int SavedFrames = 0;
	int TempBlueScore = 0;
	int TempOrangeScore = 0;

	int Tick = 8;
	int CarAmount = 0;
	int CountdownTime = 0;
	int TimeRemaining = 0;
	int OrangeScore = 0;
	int BlueScore = 0;
	int Score = 0;
	//bool isInOvertime = false;
	bool setBally = true;
	bool isInGoalReplay = false;

	std::vector<CarPosition> LastCarPosition;
	std::vector<bool> LastCarPositionSaved;
	BallPosition LastBallPosition;
	bool LastBallPositionSaved;
	

	//saves per frame
	std::vector<CarPosition> CarPositionsPerFrame;
	std::vector<BallPosition> BallPositionPerFrame;
	std::vector<GameInfo> GameInfoPerFrame;
	//std::vector<int> LobbySizePerFrame;
	BallPosition prevBallTemp;
	bool prevBallTempBool;
	int missedFrames;
	int savedBallFrames;

	//replay saves
	std::vector<int> BlueScoredFrame;
	std::vector<int> OrangeScoredFrame;

	//switch car variables
	int carit = 0;

	//JumpIn
	int BallHits = 0;
	int JumpInTick = 0;


	//Overlay
	int resX = 1920;
	int resY = 1080;

	void DrawHud(CanvasWrapper canvas);
	void DrawTime(CanvasWrapper canvas);
	void DrawPause(CanvasWrapper canvas);
	void DrawTimeline(CanvasWrapper canvas);
	void DrawTimelineSpent(CanvasWrapper canvas);
	void DrawTimelineNotConverted(CanvasWrapper canvas);
	void DrawPlayer(CanvasWrapper canvas);
	void DrawJumpIn(CanvasWrapper canvas);
	void DrawOrangeScored(CanvasWrapper canvas);
	void DrawBlueScored(CanvasWrapper canvas);
	void DrawKeyframe(CanvasWrapper canvas);

	int ConvertToScreenSizeX(int HDpixel);
	int ConvertToScreenSizeY(int HDpixel);
	float ConvertTextSizeX(float Text);
	float ConvertTextSizeY(float Text);


	//Overlay variables
	int MinutesAvailable = 0;
	int SecondsAvailable = 0;
	int MinutesDone = 0;
	int SecondsDone = 0;

private:

	void Log(std::string msg);

	// Inherited via PluginSettingsWindow
	//void RenderSettings() override;
	//std::string GetPluginName() override;

	// Inherited via PluginWindow
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "JumpInReplay";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
};

