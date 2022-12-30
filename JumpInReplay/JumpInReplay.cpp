#include "pch.h"
#include "JumpInReplay.h"
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <map>


BAKKESMOD_PLUGIN(JumpInReplay, "JumpInReplay is a bakkesmod Plugin which allows you to open replays in a private match and take control of any car in any situation", plugin_version, PLUGINTYPE_FREEPLAY)

void JumpInReplay::onLoad() {
	this->Log("JumpInReplay is a bakkesmod Plugin which allows you to record your replays and open them in Freeplay so you can take controll over a car at any time");
	this->CvarRegister();
	this->AutoConvert();
	//rendering
	gameWrapper->RegisterDrawable(bind(&JumpInReplay::DrawHud, this, std::placeholders::_1));

	return;
}

void JumpInReplay::onUnload() {
	gameWrapper->UnhookEvent("Function TAGame.Replay_TA.EventPostTimeSkip");
	return;
}


//Register All Cvars

void JumpInReplay::CvarRegister() {

	cvarManager->registerCvar("jumpIn_replaySave", "0", "Starts saving the replay for private match", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SaveReplay, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_privateMatch", "0", "Starts a private Match", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::StartPrivateMatch, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_botSpawn", "0", "Spawns right amount of bots", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SpawnBots, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_controllGame", "0", "replays the replay as JumpInReplay", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::ControllGame, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_switchPlayer", "0", "switches the player you are spectating", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SwitchCars, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_switchBack", "0", "switches back to the player you where previosly spectating", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SwitchBack, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_jumpIn", "0", "lets the player take controll of current spectated car", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::JumpIn, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_skip", "0", "skips given amount of seconds", true, true, -2000.0f, true, 2000.0f, false).addOnValueChanged(std::bind(&JumpInReplay::Skip, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_bindings", "0", "binds standard binds", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::Bindings, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_convert", "0", "converts replays into JumpInReplays", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::Convert, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_openReplay", "0", "opens saved JumpInReplay", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::OpenReplay, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_autoConvert", "0", "automaticly converts replays into JumpInReplays", true, true, 0.0f, true, 1.0f, true).addOnValueChanged(std::bind(&JumpInReplay::AutoConvertCvar, this, std::placeholders::_1, std::placeholders::_2));

	cvarManager->registerCvar("jumpIn_pause", "0", "pauses replay", true, true, 0.0f, true, 1.0f, false);
	cvarManager->registerCvar("jumpIn_inputToUnpause", "1", "unpauses replay when jumpedIn via controller input", true, true, 0.0f, true, 1.0f, true);
	cvarManager->registerCvar("jumpIn_resolution", "3", "resolution how the replay is converted (lower better Resolution->slower Conversion)", true, true, 0.01f, true, 20.0f, true);
	cvarManager->registerCvar("jumpIn_limitedBoost", "1", "limits boost in JumpInReplays", true, true, 0.0f, true, 1.0f, true);
	cvarManager->registerCvar("jumpIn_showHud", "1", "shows hud in JumpInReplays", true, true, 0.0f, true, 1.0f, true);
	cvarManager->registerCvar("jumpIn_convertKeyframes", "0", "only converts the replay for the keyframe sequences", true, true, 0.0f, true, 1.0f, true);
	cvarManager->registerCvar("jumpIn_timeBeforKeyframe", "20", "time that is converted before the keyframe", true, true, 0.0f, true, 1200.0f, true);
	cvarManager->registerCvar("jumpIn_timeAfterKeyframe", "5", "time  that is converted after the keyframe", true, true, 0.0f, true, 1200.0f, true);
	cvarManager->registerCvar("jumpIn_disableGoal", "1", "disables the goal", false, true, 0.0f, true, 1.0f, true);
	return;
}

//--------//
//FUNCTION//
//--------//


//Save Replay As JumpInReplay

void JumpInReplay::SaveReplay(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInReplay() == true) {
			if (cvar.getIntValue() == 1) {

				ReplayWrapper Replay = gameWrapper->GetGameEventAsReplay().GetReplay();
				ReplayServerWrapper ServerReplay = gameWrapper->GetGameEventAsReplay();
				ReplaySize = Replay.GetNumFrames();
				
				//get rid of old replay
				playerNames.clear();
				CarLayouts.clear();
				//StartCars.clear();
				StartTeams.clear();
				CarPositionsPerFrame.clear();
				BallPositionPerFrame.clear();
				primeColor.clear();
				//secondColor.clear();
				BlueScoredFrame.clear();
				OrangeScoredFrame.clear();
				GameInfoPerFrame.clear();
				Gamemode = 0;
				GamemodeStr = "";
				AdditionalMutators = "";
				
				//save new replay
				ReplayTick = 0;
				//Frame = 0;

				this->OneTimeSaves();
				gameWrapper->GetGameEventAsReplay().SkipToFrame(0);

				gameWrapper->HookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep", std::bind(&JumpInReplay::SaveGameState, this, std::placeholders::_1));

				this->Log("Amount of Frames in Replay " + std::to_string(ReplaySize));

			}
			else {
				gameWrapper->UnhookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep");
				gameWrapper->GetGameEventAsReplay().SetGameSpeed(1.0f);
			}
		}
		else {
			gameWrapper->UnhookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep");
			gameWrapper->GetGameEventAsReplay().SetGameSpeed(1.0f);
			Log("you must be in a replay to execute this command");
			cvar.setValue(false);
		}
	}
	return;
}

void JumpInReplay::SaveGameState(std::string eventName) {
	if (cvarManager->getCvar("jumpIn_replaySave").getBoolValue() == true) {
		if (gameWrapper->IsInReplay() == true) {
			if (Keyframes.size() > 0) {
				if (curKeyframeInConvert < Keyframes.size() && gameWrapper->GetGameEventAsReplay().GetCurrentReplayFrame() >= Keyframes.at(curKeyframeInConvert)) {
					KeyframesInConvert.push_back(ReplayTick);
					curKeyframeInConvert++;
				}
				if (cvarManager->getCvar("jumpIn_convertKeyframes").getBoolValue() == true) {
					int  frameStartKeyframe = Keyframes.at(curKeyframe) - (cvarManager->getCvar("jumpIn_timeBeforKeyframe").getIntValue() * gameWrapper->GetGameEventAsReplay().GetReplayFPS());
					if (frameStartKeyframe < 0) { frameStartKeyframe = 0; }
					int frameEndKeyframe = Keyframes.at(curKeyframe) + (cvarManager->getCvar("jumpIn_timeAfterKeyframe").getIntValue() * gameWrapper->GetGameEventAsReplay().GetReplayFPS());
					if (gameWrapper->GetGameEventAsReplay().GetCurrentReplayFrame() < frameStartKeyframe) { gameWrapper->GetGameEventAsReplay().GetReplay().SetCurrentFrame(frameStartKeyframe); }
					if (gameWrapper->GetGameEventAsReplay().GetCurrentReplayFrame() > frameEndKeyframe) {
						curKeyframe++;
						if (curKeyframe >= Keyframes.size()) {
							ReplaySize = gameWrapper->GetGameEventAsReplay().GetCurrentReplayFrame();
						}
					}
				}
			}

			if (gameWrapper->GetGameEventAsReplay().GetGameSpeed() != cvarManager->getCvar("jumpIn_resolution").getFloatValue()) {
				gameWrapper->GetGameEventAsReplay().SetGameSpeed(cvarManager->getCvar("jumpIn_resolution").getFloatValue());
			}

			//Save time and score
			SaveGameInfo();

			//BallSave
			if (gameWrapper->GetGameEventAsReplay().GetGameBalls().Count() < 1) {
				SaveBall0();
			}
			else {
				SaveBall();
			}

			ArrayWrapper<CarWrapper> Cars = gameWrapper->GetGameEventAsReplay().GetCars();

			/*//get new cars
			for (int k = 0;k < Cars.Count();k++) {
				bool alreadySaved = false;
				for (int i = 0;i < playerNames.size();i++) {
					if (playerNames.at(i) == Cars.Get(k).GetOwnerName()) { alreadySaved = true; }
				}
				if (alreadySaved == false) { 
					playerNames.push_back(Cars.Get(k).GetOwnerName());
					newCars++;
				}
			}*/

			//CarSave

			for (int k = 0;k < LobbySize;k++) {
				int i = 0;
				//bool quit = false;
				while (i < Cars.Count() && playerNames.at(k) != Cars.Get(i).GetOwnerName()) {
					i++;
				}
				if (i<Cars.Count()) {
					SaveCar(i);
				}
				else {
					SaveCar0();
				}
			}

			ReplayTick++;

			if (gameWrapper->GetGameEventAsReplay().GetCurrentReplayFrame() >= ReplaySize - 1) {
				gameWrapper->UnhookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep");
				this->Log("is unhooked");
				gameWrapper->GetGameEventAsReplay().SetGameSpeed(1.0f);
				if (cvarManager->getCvar("jumpIn_convert").getBoolValue() == true) {
					cvarManager->executeCommand("jumpIn_privateMatch 1");
				}
				cvarManager->getCvar("jumpIn_replaySave").setValue(false);
			}
		}
		else {
			gameWrapper->UnhookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep");
			this->Log("is unhooked");
			//gameWrapper->GetGameEventAsReplay().SetGameSpeed(1.0f);
			if (cvarManager->getCvar("jumpIn_convert").getBoolValue() == true) {
				cvarManager->getCvar("jumpIn_convert").setValue(false);
			}
			cvarManager->getCvar("jumpIn_replaySave").setValue(false);
		}
	}

	return;
}

void JumpInReplay::SaveCar(int i) {
	CarWrapper Car = gameWrapper->GetGameEventAsReplay().GetCars().Get(i);
	CarPosition CarTemp;
	CarTemp.location = Car.GetLocation();
	CarTemp.velocitiy = Car.GetVelocity();
	CarTemp.rotation = Car.GetRotation();
	CarTemp.angVelocity = Car.GetAngularVelocity();
	CarTemp.steer = Car.GetReplicatedSteer();
	CarTemp.throttle = Car.GetReplicatedThrottle();
	CarTemp.handbrake = Car.GetbReplicatedHandbrake();
	CarTemp.boostAmount = Car.GetBoostComponent().IsNull() ? 0 : Car.GetBoostComponent().GetCurrentBoostAmount();
	CarTemp.isBoosting = Car.IsBoostCheap();
	//CarTemp.lastJumped = !Car.GetbJumped() || Car.GetJumpComponent().IsNull() ? -1 : Car.GetJumpComponent().GetInactiveTime();
	//CarTemp.hasDodge = !Car.GetbDoubleJumped() && CarTemp.lastJumped < 1.2f;
	CarTemp.ballTouches = Car.GetPRI().GetBallTouches();
	CarTemp.team = Car.GetTeamNum2();
	CarTemp.isActive = true;
	//CarTemp.name = Car.GetOwnerName();
	CarPositionsPerFrame.push_back(CarTemp);

	//Log(std::to_string(i) + ": " + std::to_string(Car.GetReplicatedSteer()));
}

void JumpInReplay::SaveCar0() {
	CarPosition CarTemp1;
	CarTemp1.location = Vector(0, 0, 2200);
	CarTemp1.velocitiy = Vector(0, 0, 0);
	CarTemp1.rotation = Rotator(0, 0, 0);
	CarTemp1.angVelocity = Vector(0, 0, 0);
	CarTemp1.steer = 0.0f;
	CarTemp1.throttle = 0.0f;
	CarTemp1.handbrake = false;
	CarTemp1.boostAmount = 0.0f;
	CarTemp1.isBoosting = false;
	//CarTemp1.hasDodge = false;
	CarTemp1.ballTouches = 0;
	CarTemp1.team = 0;
	/*if (ReplayTick - 1 <= 0) { CarTemp1.name = ""; }
	else { CarTemp1.name = CarPositionsPerFrame.at(ReplayTick - 1).name; }*/
	CarTemp1.isActive = false;
	CarPositionsPerFrame.push_back(CarTemp1);
}

void JumpInReplay::SaveBall() {
	BallWrapper Ball = gameWrapper->GetGameEventAsReplay().GetBall();
	BallPosition BallTemp;
	BallTemp.location = Ball.GetLocation();
	BallTemp.velocitiy = Ball.GetVelocity();
	BallTemp.rotation = Ball.GetRotation();
	BallTemp.angVelocity = Ball.GetAngularVelocity();
	BallPositionPerFrame.push_back(BallTemp);
}

void JumpInReplay::SaveBall0() {
	BallPosition BallTemp;
	BallTemp.location = Vector(0, 0, 0);
	BallTemp.velocitiy = Vector(0, 0, 0);
	BallTemp.rotation = Rotator(0, 0, 0);
	BallTemp.angVelocity = Vector(0, 0, 0);
	BallPositionPerFrame.push_back(BallTemp);
}

void JumpInReplay::SaveGameInfo() {
	ReplayServerWrapper Game = gameWrapper->GetGameEventAsReplay();
	GameInfo GameInfoTemp;
	GameInfoTemp.BlueScore = Game.GetTeams().Get(0).GetScore();
	GameInfoTemp.OrangeScore = Game.GetTeams().Get(1).GetScore();
	GameInfoTemp.Countdowntime = Game.GetReplicatedRoundCountDownNumber();
	GameInfoTemp.TimeRemaining = Game.GetSecondsRemaining();
	GameInfoPerFrame.push_back(GameInfoTemp);
}


void JumpInReplay::OneTimeSaves() {
	if (gameWrapper->IsInReplay() == true) {
		ReplayServerWrapper Replay = gameWrapper->GetGameEventAsReplay();
		//general saves
		LobbySize = Replay.GetCars().Count();
		Arena = gameWrapper->GetCurrentMap();
		Gamemode = gameWrapper->GetGameEventAsReplay().GetPlaylist().GetPlaylistId();

		//Keyframes save
		temppath = gameWrapper->GetBakkesModPath().string() + "/data/jumpInReplay/current.replay";
		this->Log("temporary replay saved to: " + temppath);
		Replay.GetReplay().ExportReplay(temppath);
		this->KeyframeSaves();
		ReplayFPS = gameWrapper->GetGameEventAsReplay().GetReplayFPS();

		LinearColor orange = gameWrapper->GetGameEventAsReplay().GetTeams().Get(1).GetPrimaryColor();
		LinearColor blue = gameWrapper->GetGameEventAsReplay().GetTeams().Get(0).GetPrimaryColor();

		//player specific saves
		for (int i = 0;i < LobbySize;i++) {
			playerNames.push_back(Replay.GetCars().Get(i).GetOwnerName());
			//StartCars.push_back(Replay.GetCars().Get(i).GetbLoadoutSet());
			CarLayouts.push_back(Replay.GetCars().Get(i).GetLoadoutBody());
			StartTeams.push_back((Replay.GetCars().Get(i).GetTeamNum2()));
			//save color
			if (StartTeams.at(i) == 0) {
				primeColor.push_back(blue);
				//secondColor.push_back(blue);
			}
			else {
				primeColor.push_back(orange);
				//secondColor.push_back(orange);
			}
		}
		hasReplaySaved = true;
		this->Log("OneTimeSaves got saved");
	}
}


void JumpInReplay::KeyframeSaves() {
	this->Keyframes.clear();
	this->KeyframesInConvert.clear();
	curKeyframe = 0;
	curKeyframeInConvert = 0;
	//open temp file
	std::string ReplayAsString;
	std::ifstream ReplayFile(temppath,std::ios_base::out | std::ios_base::binary);
	if (!ReplayFile.is_open()) { this->Log("can't find temp file"); return; }
	std::stringstream buffer;
	buffer << ReplayFile.rdbuf();
	ReplayAsString = buffer.str();
	//convert to hex
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (size_t i = 0; ReplayAsString.length() > i; ++i) {
		ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(ReplayAsString[i]));
	}
	std::string ReplayAsHex = ss.str();
	//search for keyframes
	std::vector<std::string> KeyframesInHex;
	std::size_t stringpos = 0;
	while (stringpos <= ReplayAsHex.size()) {
		stringpos = ReplayAsHex.find("005573657200", stringpos + 1);
		if (stringpos != std::string::npos) {
			KeyframesInHex.push_back(ReplayAsHex.substr(stringpos+12, 4));
		}
		else {
			this->Log("no more keyframes found");
			break;
		}
	}
	//convert to int
	for (int k = 0;k<KeyframesInHex.size();k++) {
		//small endian to big endian
		std::string SmallEndian = KeyframesInHex.at(k);
		KeyframesInHex.at(k) = SmallEndian.substr(2, 2) + SmallEndian.substr(0, 2);
		//intinfy
		Keyframes.push_back(std::stoul(KeyframesInHex.at(k), nullptr, 16));
	}
	return;
}

//Start Private Match

void JumpInReplay::StartPrivateMatch(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (hasReplaySaved == true) {
			//sets gamemode
			if (Gamemode < 14 && Gamemode>0) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }
			else if(Gamemode==16 || Gamemode==22 || Gamemode==34 || Gamemode==35){ GamemodeStr = "Soccar"; AdditionalMutators = ""; }//normal
			else if (Gamemode == 17 || Gamemode == 27) { GamemodeStr = "Basketball"; AdditionalMutators = ""; }//hoops
			else if (Gamemode == 15 || Gamemode == 30) { GamemodeStr = "Hockey"; AdditionalMutators = ""; }//snowday
			else if (Gamemode == 46) { GamemodeStr = "Football"; AdditionalMutators = ""; }//american football
			else if (Gamemode == 38 || Gamemode == 43) { GamemodeStr = "GodBall"; AdditionalMutators = ""; }//heatseaker
			//not fully implemnted yet
			else if (Gamemode == 18 || Gamemode == 28) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }//rumble
			else if (Gamemode == 29) { GamemodeStr = "Breakout"; AdditionalMutators = ""; }//dropshot
			else if (Gamemode == 37) { GamemodeStr = "Breakout"; AdditionalMutators = ""; }//dropshot rumble
			//not implemented yet
			else if (Gamemode == 41) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }//boomer
			else if (Gamemode == 47) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }//super cube
			else if (Gamemode == 33) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }//spike rush
			else if (Gamemode == 31) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }//ghost hunt
			else if (Gamemode == 44) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }//winter brakeaway
			else if (Gamemode == 32) { GamemodeStr = "Soccar"; AdditionalMutators = ""; }//beach ball
			else{ GamemodeStr = "Soccar"; AdditionalMutators = ""; }//normal
			//enable goal?
			if (cvarManager->getCvar("jumpIn_disableGoal").getBoolValue() == true) {
				AdditionalMutators = AdditionalMutators + ",DisableGoalDelay";
			}
			//opens private match
			cvarManager->executeCommand("unreal_command 'open " + Arena + "?Game=TAGame.GameInfo_" + GamemodeStr + "_TA?Offline?Gametags=noDemolish,UnlimitedTime"+AdditionalMutators+"'");


		}
		cvar.setValue(false);
	}
}


//Bot Spawning

void JumpInReplay::SpawnBots(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && hasReplaySaved == true && gameWrapper->IsInFreeplay() == false) {
			this->Log(std::to_string(LobbySize));
			carit = 0;
			SavedFrames = ReplayTick;
			//set team player
			gameWrapper->GetGameEventAsServer().GetPRIs().Get(0).ServerChangeTeam(StartTeams.at(carit));
			//only Bots
			for (int i = 0;i < LobbySize;i++) {
				SpawnOneBot(i);
			}
			//every Car
			for (int i = 0;i < LobbySize;i++) {
				this->Log(playerNames.at(i));
			}
			
			gameWrapper->GetGameEventAsServer().GetCars().Get(carit + 1).SetHidden2(true);

			//sets boost limited
			if (cvarManager->getCvar("jumpIn_limitedBoost").getBoolValue() == true) {
				cvarManager->executeCommand("boost set limited", false);
			}
			else {
				cvarManager->executeCommand("boost set unlimited", false);
			}
			//open or converts replay
			if (cvarManager->getCvar("jumpIn_convert").getBoolValue() == true || cvarManager->getCvar("jumpIn_openReplay").getBoolValue() == true) {
				gameWrapper->SetTimeout([this](GameWrapper* gw) {
					cvarManager->executeCommand("jumpIn_controllGame 1", true);
					}, 2.0f);
				cvarManager->getCvar("jumpIn_convert").setValue(false);
				cvarManager->getCvar("jumpIn_openReplay").setValue(false);
			}
		}
		cvar.setValue(false);
	}
}

void JumpInReplay::SpawnOneBot(int i) {
	gameWrapper->GetCurrentGameState().SetbRandomizedBotLoadouts(false);
	gameWrapper->GetGameEventAsServer().SpawnBot(CarLayouts.at(i), playerNames.at(i));
}


//Controll the game

void JumpInReplay::ControllGame(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && hasReplaySaved == true && gameWrapper->IsInFreeplay() == false) {
			//reset values
			cvarManager->getCvar("jumpIn_jumpIn").setValue(0);
			cvarManager->getCvar("jumpIn_pause").setValue(0);
			Tick = 0;
			carit = 0;
			CountdownTime = 0;
			//isInOvertime = false;
			OrangeScore = 0;
			BlueScore = 0;
			TimeRemaining = 0;

			isInGoalReplay = false;
			//hook for goal replays
			if (cvarManager->getCvar("jumpIn_disableGoal").getBoolValue() == false) {
				gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", std::bind(&JumpInReplay::GoalReplayStart, this, std::placeholders::_1));
				gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", std::bind(&JumpInReplay::GoalReplayEnd, this, std::placeholders::_1));
			}

			//calculate goals frames
			for (int i = 1;i < SavedFrames;i++) {
				if (GameInfoPerFrame.at(i).BlueScore != GameInfoPerFrame.at(i - 1).BlueScore) { BlueScoredFrame.push_back(i); }
				if (GameInfoPerFrame.at(i).OrangeScore != GameInfoPerFrame.at(i - 1).OrangeScore) { OrangeScoredFrame.push_back(i); }
			}

			gameWrapper->GetGameEventAsServer().SetLobbyEndCountdown(0);
			CarAmount = gameWrapper->GetGameEventAsServer().GetCars().Count();
			gameWrapper->HookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep", std::bind(&JumpInReplay::ControllGamePerTick, this, std::placeholders::_1));

			//set lobby color and team
			gameWrapper->SetTimeout([this](GameWrapper* gw) {
				for (int i = 0;i < LobbySize;i++) {
					for (int p = 1;p < gameWrapper->GetGameEventAsServer().GetCars().Count();p++) {
						//set bot teams and car color
						if (playerNames.at(i) == gameWrapper->GetGameEventAsServer().GetCars().Get(p).GetOwnerName()) {
							//gameWrapper->GetGameEventAsServer().GetCars().Get(p).GetPRI().ServerChangeTeam(StartTeams.at(i));
							gameWrapper->GetGameEventAsServer().GetCarArchetype();
							
							gameWrapper->GetGameEventAsServer().GetCars().Get(p).GetPRI().SetPlayerTeam(gameWrapper->GetGameEventAsServer().GetTeams().Get(StartTeams.at(i)));
							//gameWrapper->GetGameEventAsServer().GetCars().Get(p).SetCarColor(primeColor.at(i), secondColor.at(i));
						}
						//set player car color and team
						//gameWrapper->GetGameEventAsServer().GetCars().Get(0).GetPRI().ServerChangeTeam(StartTeams.at(carit));
						gameWrapper->GetGameEventAsServer().GetCars().Get(0).GetPRI().SetPlayerTeam(gameWrapper->GetGameEventAsServer().GetTeams().Get(StartTeams.at(carit)));//sets team for player
						//gameWrapper->GetGameEventAsServer().GetCars().Get(0).SetCarColor(primeColor.at(carit), secondColor.at(carit));//sets color for car of player
					}
				}
				}, 0.5f);
		}
		else {
			gameWrapper->UnhookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep");
			cvar.setValue(false);
		}
	}
}

void JumpInReplay::ControllGamePerTick(std::string eventName) {
	if (cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true && gameWrapper->IsInGame() == true && gameWrapper->IsInFreeplay() == false) {
		if (Tick < (SavedFrames - 8)) {
			if (isInGoalReplay == false) {
				//cars
				for (int i = 1;i < LobbySize + 1;i++) {
					//car switching and setting cars
					if (i - 1 == carit) {
						SetCar0(i);
						//jump In
						if (cvarManager->getCvar("jumpIn_pause").getBoolValue() == false && cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() == true && GameInfoPerFrame.at(Tick).Countdowntime == 0) {
							gameWrapper->GetGameEventAsServer().GetCars().Get(0).ForceBoost(false);
							gameWrapper->GetGameEventAsServer().GetCars().Get(0).SetbOverrideHandbrakeOn(false);
							if (cvarManager->getCvar("jumpIn_limitedBoost").getBoolValue() == false) { gameWrapper->GetGameEventAsServer().GetCars().Get(0).GetBoostComponent().SetBoostAmount(1.0f); }
						}
						else {
							SetCar(0, i - 1);
						}
					}
					else {
						SetCar(i, i - 1);
					}
				}


				//ball
				//jumpin ball hit
				if (gameWrapper->GetGameEventAsServer().GetPRIs().Get(0).GetBallTouches() > BallHits && cvarManager->getCvar("jumpIn_pause").getBoolValue() == false && cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() == true) {
					setBally = false;
				}
				//jumpin ghosthit exemption
				if (CarPositionsPerFrame.at(((Tick + 8) * LobbySize) + carit).ballTouches != CarPositionsPerFrame.at(((Tick + 7) * LobbySize) + carit).ballTouches && cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() == true) {
					setBally = false;
				}
				//score exemption
				if (Score != gameWrapper->GetGameEventAsServer().GetTotalScore()) {
					setBally = false;
				}
				if (GameInfoPerFrame.at(Tick).Countdowntime != CountdownTime) {
					setBally = true;
				}
				//set ball
				if (setBally == true) {
					SetBall();
					//Log("X:" + std::to_string(BallPositionPerFrame.at(Tick).location.X) + " Y:" + std::to_string(BallPositionPerFrame.at(Tick).location.Z));
				}

				//score and time
				SetGameInfo();

				//variable incrementation
				Score = gameWrapper->GetGameEventAsServer().GetTotalScore();
				CountdownTime = GameInfoPerFrame.at(Tick).Countdowntime;

				//pause
				if (cvarManager->getCvar("jumpIn_pause").getBoolValue() == false) {
					Tick++;
				}
				//reset pause
				else {
					if (cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() == true) {
						//unpause with controller input
						if (cvarManager->getCvar("jumpIn_inputToUnpause").getBoolValue() == true) {
							auto CarInput = gameWrapper->GetPlayerController().GetVehicleInput();
							if (CarInput.HoldingBoost == true || CarInput.Throttle < -0.1 || CarInput.Throttle > 0.1) {
								cvarManager->executeCommand("jumpIn_pause 0");
							}
						}
						Tick = JumpInTick;
						BallHits = gameWrapper->GetGameEventAsServer().GetPRIs().Get(0).GetBallTouches();
						setBally = true;
						gameWrapper->GetGameEventAsServer().ResetPickups();
					}
				}
			}
		}
		else {
			Tick = 0;
		}
	}
	else {
		Tick = 0;
		gameWrapper->UnhookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep");
		cvarManager->getCvar("jumpIn_controllGame").setValue(false);
	}
}

void JumpInReplay::SetCar(int i, int it) {
	CarWrapper Car = gameWrapper->GetGameEventAsServer().GetCars().Get(i);
	if (i == 0) {
		static ControllerInput input;
		input.Throttle = (CarPositionsPerFrame.at((Tick * LobbySize) + it).throttle - 128.0) / 128.0;
		input.Steer = (CarPositionsPerFrame.at((Tick * LobbySize) + it).steer - 128.0) / 128.0;
	}

	Car.SetLocation(CarPositionsPerFrame.at((Tick * LobbySize) + it).location);
	Car.SetVelocity(CarPositionsPerFrame.at((Tick * LobbySize) + it).velocitiy);
	Car.SetRotation(CarPositionsPerFrame.at((Tick * LobbySize) + it).rotation);
	Car.SetAngularVelocity(CarPositionsPerFrame.at((Tick * LobbySize) + it).angVelocity, false);
	Car.SetbOverrideHandbrakeOn(CarPositionsPerFrame.at((Tick * LobbySize) + it).handbrake);
	Car.GetBoostComponent().SetBoostAmount(CarPositionsPerFrame.at((Tick * LobbySize) + it).boostAmount);
	Car.ForceBoost(CarPositionsPerFrame.at((Tick * LobbySize) + it).isBoosting);
	Car.GetPRI().SetPlayerTeam(gameWrapper->GetGameEventAsServer().GetTeams().Get(CarPositionsPerFrame.at((Tick * LobbySize) + it).team));
	//Car.GetPRI().eventSetPlayerName(CarPositionsPerFrame.at((Tick * LobbySize) + it).name);
	//Car.SetHidden2(false);
	//Car.SetbCanJump(CarPositionsPerFrame.at((Tick * LobbySize) + it).hasDodge);

	//Log(std::to_string(i)+": "+std::to_string(CarPositionsPerFrame.at(((Tick) * LobbySize) + it).location.X) + ", " + std::to_string(CarPositionsPerFrame.at(((Tick) * LobbySize) + it).location.Y) + ", " + std::to_string(CarPositionsPerFrame.at(((Tick) * LobbySize) + it).location.Z));
}

void JumpInReplay::SetCar0(int i) {
	CarWrapper Car = gameWrapper->GetGameEventAsServer().GetCars().Get(i);
	
	Car.SetLocation(Vector(0, 0, 5000));
	Car.SetVelocity(Vector(0, 0, 0));
	Car.SetRotation(Rotator(0, 0, 0));
	Car.SetAngularVelocity(Vector(0, 0, 0), false);
	//Car.GetBoostComponent().SetBoostAmount();
	Car.ForceBoost(0);
	//Car.SetHidden2(true);
	//Car.SetbCanJump(0);
}

void JumpInReplay::SetBall() {
	BallWrapper Ball = gameWrapper->GetGameEventAsServer().GetBall();
	Ball.SetLocation(BallPositionPerFrame.at(Tick).location);
	Ball.SetVelocity(BallPositionPerFrame.at(Tick).velocitiy);
	Ball.SetRotation(BallPositionPerFrame.at(Tick).rotation);
	Ball.SetAngularVelocity(BallPositionPerFrame.at(Tick).angVelocity, false);
}

void JumpInReplay::SetGameInfo() {
	gameWrapper->GetGameEventAsServer().SetScoreAndTime(gameWrapper->GetGameEventAsServer().GetLocalPrimaryPlayer(), GameInfoPerFrame.at(Tick).BlueScore, GameInfoPerFrame.at(Tick).OrangeScore, GameInfoPerFrame.at(Tick).TimeRemaining, false, false);

	if (GameInfoPerFrame.at(Tick).Countdowntime != CountdownTime) {
		if (GameInfoPerFrame.at(Tick).Countdowntime == 0) {
			gameWrapper->GetGameEventAsServer().BroadcastGoMessage();
		}
		else {
			gameWrapper->GetGameEventAsServer().BroadcastCountdownMessage(GameInfoPerFrame.at(Tick).Countdowntime);
		}
	}
}

//hooks for goal scoring enabled
void JumpInReplay::GoalReplayStart(std::string eventName) {
	isInGoalReplay = true;
}

void JumpInReplay::GoalReplayEnd(std::string eventName) {
	isInGoalReplay = false;
}


//skip

void JumpInReplay::Skip(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getIntValue() != 0) {
		if (gameWrapper->IsInGame() == true && cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true) {
			setBally = true;
			if (Tick < 150 && Tick >= 0) { Tick = (SavedFrames - 8) + 120 * cvar.getFloatValue(); }
			else {
				Tick = Tick + (120 * cvar.getFloatValue());
			}
			if (Tick < 0) { Tick = 0; }
			if (Tick > SavedFrames - 8) { Tick = 0; }
		}
		cvar.setValue(0);
	}
}


//car switching

void JumpInReplay::SwitchCars(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true) {
			ServerWrapper Game = gameWrapper->GetGameEventAsServer();
			while (CarPositionsPerFrame.at((Tick * LobbySize) + carit).isActive==false)
			{
				carit++;
			}
			Game.GetCars().Get(carit + 1).SetHidden2(false);
			carit++;
			if (carit > (LobbySize - 1)) { carit = 0; }
			gameWrapper->GetGameEventAsServer().GetCars().Get(0).GetPRI().SetPlayerTeam(gameWrapper->GetGameEventAsServer().GetTeams().Get(StartTeams.at(carit)));//sets team for player
			//gameWrapper->GetGameEventAsServer().GetCars().Get(0).SetCarColor(primeColor.at(carit), secondColor.at(carit));//sets color for car of player
			Game.GetCars().Get(carit + 1).SetHidden2(true);
		}
		cvar.setValue(false);
	}
}

void JumpInReplay::SwitchBack(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true) {
			for (int i = 0;i < (LobbySize - 1);i++) {
				cvarManager->executeCommand("jumpIn_switchPlayer 1", false);
			}
		}
		cvar.setValue(false);
	}
}


//jump In

void JumpInReplay::JumpIn(std::string oldValue, CVarWrapper cvar) {
	if (gameWrapper->IsInGame() == true && cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true) {
		BallHits = gameWrapper->GetGameEventAsServer().GetPRIs().Get(0).GetBallTouches();
		if (cvar.getIntValue() == 1) {
			cvarManager->executeCommand("jumpIn_pause 1");
			gameWrapper->GetGameEventAsServer().ResetPickups();
			setBally = true;
			JumpInTick = Tick;
			//set boost because airdribble plugin fuck my shit up like crazy and isn't even open source AAAAAAAHHHHH
			cvarManager->executeCommand("sv_soccar_enablegoal 0", false);
			if (cvarManager->getCvar("jumpIn_limitedBoost").getBoolValue() == true) {
				cvarManager->executeCommand("boost set limited", false);
			}
			else {
				cvarManager->executeCommand("boost set unlimited", false);
			}
		}
		else {
			setBally = true;
			Tick = JumpInTick;
			cvarManager->executeCommand("jumpIn_pause 1");
		}
	}
}


//convert

//manual
void JumpInReplay::Convert(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInReplay() == true) {
			cvarManager->executeCommand("jumpIn_replaySave 1", true);
		}
		gameWrapper->HookEvent("Function Engine.GameInfo.InitGame", std::bind(&JumpInReplay::JoinedFreeplay, this, std::placeholders::_1));
	}
}

void JumpInReplay::JoinedFreeplay(std::string eventName) {
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.SetGameEvent", std::bind(&JumpInReplay::BallSpawned, this, std::placeholders::_1));
	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		cvarManager->executeCommand("jumpIn_botSpawn 1", true);
		}, 3.0f);
	gameWrapper->UnhookEvent("Function Engine.GameInfo.InitGame");
}


//auto
void JumpInReplay::AutoConvertCvar(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		gameWrapper->HookEvent("Function TAGame.GameInfo_Replay_TA.InitGame", std::bind(&JumpInReplay::IsInReplay, this, std::placeholders::_1));
		cvarManager->executeCommand("jumpIn_convert 1", false);
	}
	else {
		gameWrapper->UnhookEvent("Function TAGame.GameInfo_Replay_TA.InitGame");
		cvarManager->getCvar("jumpIn_convert").setValue(false);
	}
}

void JumpInReplay::AutoConvert() {
	if (cvarManager->getCvar("jumpIn_autoConvert").getBoolValue() == true) {
		gameWrapper->HookEvent("Function TAGame.GameInfo_Replay_TA.InitGame", std::bind(&JumpInReplay::IsInReplay, this, std::placeholders::_1));
	}
	else {
		gameWrapper->UnhookEvent("Function TAGame.GameInfo_Replay_TA.InitGame");
	}
}

void JumpInReplay::IsInReplay(std::string eventName) {
	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		cvarManager->executeCommand("jumpIn_convert 1", true);
		}, 4.0f);
}


void JumpInReplay::OpenReplay(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (hasReplaySaved == true) {
			cvarManager->getCvar("jumpIn_controllGame").setValue(false);
			cvarManager->executeCommand("jumpIn_privateMatch 1", true);
			gameWrapper->HookEvent("Function Engine.GameInfo.InitGame", std::bind(&JumpInReplay::JoinedFreeplay, this, std::placeholders::_1));
		}
	}
}


//bindings

void JumpInReplay::Bindings(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		//controller
		cvarManager->executeCommand("bind XboxTypeS_DPad_Right 'default_right; jumpIn_skip 10'");
		cvarManager->executeCommand("bind XboxTypeS_DPad_Left 'default_left; jumpIn_skip -5'");
		cvarManager->executeCommand("bind XboxTypeS_DPad_Up 'default_up; jumpIn_switchPlayer 1'");
		cvarManager->executeCommand("bind XboxTypeS_DPad_Down 'default_down; jumpIn_switchBack 1'");
		cvarManager->executeCommand("bind XboxTypeS_Back 'toggle jumpIn_jumpIn 1 0'");
		cvarManager->executeCommand("bind XboxTypeS_LeftThumbStick 'toggle jumpIn_pause 1 0'");
		//keyboard
		cvarManager->executeCommand("bind Right 'jumpIn_skip 10'");
		cvarManager->executeCommand("bind Left 'jumpIn_skip -5'");
		cvarManager->executeCommand("bind Up 'jumpIn_switchPlayer 1'");
		cvarManager->executeCommand("bind Down 'jumpIn_switchBack 1'");
		cvarManager->executeCommand("bind V 'toggle jumpIn_jumpIn 1 0'");
		cvarManager->executeCommand("bind B 'toggle jumpIn_pause 1 0'");

		cvar.setValue(false);
	}
}


//-------//
//OVERLAY//
//-------//


//draw HUD

void JumpInReplay::DrawHud(CanvasWrapper canvas) {
	if (cvarManager->getCvar("jumpIn_showHud").getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && hasReplaySaved == true && cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true) {
			resX = canvas.GetSize().X;
			resY = canvas.GetSize().Y;
			DrawTime(canvas);
			DrawPause(canvas);
			DrawTimeline(canvas);
			DrawTimelineSpent(canvas);
			DrawPlayer(canvas);
			DrawJumpIn(canvas);
			DrawBlueScored(canvas);
			DrawOrangeScored(canvas);
			DrawKeyframe(canvas);
		}
	}
}


//time

void JumpInReplay::DrawTime(CanvasWrapper canvas) {
	//calculate minutes and seconds
	MinutesAvailable = SavedFrames / (60 * 120);
	SecondsAvailable = (SavedFrames / 120) - (MinutesAvailable * 60);
	MinutesDone = Tick / 7200;
	SecondsDone = (Tick / 120) - (MinutesDone * 60);
	//make seconds two didgets
	std::stringstream SSecondsDone;
	SSecondsDone << std::setw(2) << std::setfill('0') << SecondsDone;
	std::string StringSecondsDone = SSecondsDone.str();
	std::stringstream SSecondsAvailable;
	SSecondsAvailable << std::setw(2) << std::setfill('0') << SecondsAvailable;
	std::string StringSecondsAvailable = SSecondsAvailable.str();
	//draw time
	canvas.SetColor(236, 236, 236, 255);
	canvas.SetPosition(Vector2{ ConvertToScreenSizeX(1361),ConvertToScreenSizeY(1000) });
	canvas.DrawString(std::to_string(MinutesDone) + ":" + StringSecondsDone + "/" + std::to_string(MinutesAvailable) + ":" + StringSecondsAvailable, ConvertTextSizeX(1.5), ConvertTextSizeY(1.5), true, false);
}


//pause

void JumpInReplay::DrawPause(CanvasWrapper canvas) {
	LinearColor color;
	color.R = 236;
	color.G = 236;
	color.B = 236;
	color.A = 255;
	if (cvarManager->getCvar("jumpIn_pause").getBoolValue() == true) {
		//triangle
		canvas.FillTriangle(Vector2{ ConvertToScreenSizeX(460),ConvertToScreenSizeY(1000) }, Vector2{ ConvertToScreenSizeX(460),ConvertToScreenSizeY(1024) }, Vector2{ ConvertToScreenSizeX(484),ConvertToScreenSizeY(1012) }, color);
	}
	else {
		//pause button
		canvas.SetColor(color);
		canvas.DrawLine(Vector2{ ConvertToScreenSizeX(460),ConvertToScreenSizeY(1000) }, Vector2{ ConvertToScreenSizeX(468),ConvertToScreenSizeY(1000) }, ConvertToScreenSizeY(24));
		canvas.DrawLine(Vector2{ ConvertToScreenSizeX(476),ConvertToScreenSizeY(1000) }, Vector2{ ConvertToScreenSizeX(484),ConvertToScreenSizeY(1000) }, ConvertToScreenSizeY(24));
	}
}


//Timeline

void JumpInReplay::DrawTimeline(CanvasWrapper canvas) {
	canvas.SetColor(232, 232, 232, 150);
	canvas.DrawLine(Vector2{ ConvertToScreenSizeX(460),ConvertToScreenSizeY(960) }, Vector2{ ConvertToScreenSizeX(1460),ConvertToScreenSizeY(960) }, ConvertToScreenSizeY(6));
}

void JumpInReplay::DrawTimelineSpent(CanvasWrapper canvas) {
	int length = 0;
	float ReplaySpent = Tick;
	float ReplayLength = SavedFrames;
	length = ConvertToScreenSizeX(1000) * (ReplaySpent / ReplayLength);
	canvas.SetColor(0, 126, 255, 230);
	canvas.DrawLine(Vector2{ ConvertToScreenSizeX(460),ConvertToScreenSizeY(960) }, Vector2{ (ConvertToScreenSizeX(460) + length),ConvertToScreenSizeY(960) }, ConvertToScreenSizeY(6));
}


//Player

void JumpInReplay::DrawPlayer(CanvasWrapper canvas) {

	//team color
	if ((static_cast<int>(StartTeams.at(carit))) == 0) {
		canvas.SetColor(12, 136, 252, 255);//blue
	}
	else {
		canvas.SetColor(252, 124, 12, 255);//orange
	}
	canvas.SetPosition(Vector2{ ConvertToScreenSizeX(650),ConvertToScreenSizeY(920) });
	canvas.DrawString(playerNames.at(carit), ConvertTextSizeX(1.5), ConvertTextSizeY(1.5), true, false);

}


//JumpIn

void JumpInReplay::DrawJumpIn(CanvasWrapper canvas) {
	canvas.SetPosition(Vector2{ ConvertToScreenSizeX(1240),ConvertToScreenSizeY(920) });
	if (cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() != true) {
		//jumpout
		canvas.SetColor(114, 225, 78, 255);
		canvas.DrawString("JumpIn", ConvertTextSizeX(1.5), ConvertTextSizeY(1.5), true, false);
	}
	else {
		//jumpIn
		canvas.SetColor(255, 1, 1, 255);
		canvas.DrawString("JumpOut", ConvertTextSizeX(1.5), ConvertTextSizeY(1.5), true, false);
	}
}


//show goals in Timelin

void JumpInReplay::DrawBlueScored(CanvasWrapper canvas) {
	LinearColor color = { 12, 136, 252, 255 };
	for (int i = 0;i < BlueScoredFrame.size();i++) {
		float ReplayLength = SavedFrames;
		float FrameScored = BlueScoredFrame.at(i);
		int position = ConvertToScreenSizeX(460) + (ConvertToScreenSizeX(1000) * (FrameScored / ReplayLength));
		canvas.FillTriangle(Vector2{ position,ConvertToScreenSizeY(963) }, Vector2{ position - ConvertToScreenSizeX(3),ConvertToScreenSizeY(972) }, Vector2{ position + ConvertToScreenSizeX(3),ConvertToScreenSizeY(972) }, color);
	}
}

void JumpInReplay::DrawOrangeScored(CanvasWrapper canvas) {
	LinearColor color = { 252, 124, 12, 255 };
	for (int i = 0;i < OrangeScoredFrame.size();i++) {
		float ReplayLength = SavedFrames;
		float FrameScored = OrangeScoredFrame.at(i);
		int position = ConvertToScreenSizeX(460) + (ConvertToScreenSizeX(1000) * (FrameScored / ReplayLength));
		canvas.FillTriangle(Vector2{ position,ConvertToScreenSizeY(963) }, Vector2{ position - ConvertToScreenSizeX(3),ConvertToScreenSizeY(972) }, Vector2{ position + ConvertToScreenSizeX(3),ConvertToScreenSizeY(972) }, color);
	}
}

void JumpInReplay::DrawKeyframe(CanvasWrapper canvas) {
	LinearColor color = { 190, 190, 190, 255 };
	for (int i = 0;i < KeyframesInConvert.size();i++) {
		float ReplayLength = SavedFrames;
		float KeyFrame = KeyframesInConvert.at(i);
		int position = ConvertToScreenSizeX(460) + (ConvertToScreenSizeX(1000) * (KeyFrame / ReplayLength));
		canvas.FillTriangle(Vector2{ position,ConvertToScreenSizeY(963) }, Vector2{ position - ConvertToScreenSizeX(3),ConvertToScreenSizeY(972) }, Vector2{ position + ConvertToScreenSizeX(3),ConvertToScreenSizeY(972) }, color);
	}
}


//convert from 1080p to all resolutions

int JumpInReplay::ConvertToScreenSizeY(int HDpixel) {
	float position = std::roundf((HDpixel / 1080.0f) *  resY);

	return static_cast<int>(position);
}

int JumpInReplay::ConvertToScreenSizeX(int HDpixel) {
	float position = std::roundf((HDpixel / 1920.0f) * resX);

	return static_cast<int>(position);
}

float JumpInReplay::ConvertTextSizeY(float Text) {
	float scale = (Text / 1080.0f) * resY;

	return scale;
}

float JumpInReplay::ConvertTextSizeX(float Text) {
	float scale = (Text / 1920.0f) * resX;

	return scale;
}


//Bakkesmod Log

void JumpInReplay::Log(std::string msg) {
	cvarManager->log(msg);

	return;
}
