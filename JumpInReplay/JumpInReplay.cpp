#include "pch.h"
#include "JumpInReplay.h"
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <map>



BAKKESMOD_PLUGIN(JumpInReplay, "JumpInReplay", "1.0", PERMISSION_ALL)

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
	
	cvarManager->registerCvar("jumpIn_replaySave", "0", "Starts saving replay for private match", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SaveReplay, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_privateMatch", "0", "Starts a private Match", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::StartPrivateMatch, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_botSpawn", "0", "Spawns right amount of bots", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SpawnBots, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_controllGame", "0", "replays the replay as JumpInReplay", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::ControllGame, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_switchPlayer", "0", "switches player you are spectating", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SwitchCars, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_switchBack","0","switches back to the player you where previosly spectating", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::SwitchBack, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_jumpIn", "0", "lets player take controll of current spectated car", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::JumpIn, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_skip", "0", "skipps given amount of seconds", true, true, -2000.0f, true, 2000.0f, false).addOnValueChanged(std::bind(&JumpInReplay::Skip, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_bindings", "0", "binds standard binds", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::Bindings, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_convert", "0", "converts replays into JumpInReplays", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::Convert, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_openReplay", "0", "opens saved JumpInReplay", true, true, 0.0f, true, 1.0f, false).addOnValueChanged(std::bind(&JumpInReplay::OpenReplay, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("jumpIn_autoConvert", "0", "automaticly converts replays into JumpInReplays", true, true, 0.0f, true, 1.0f, true).addOnValueChanged(std::bind(&JumpInReplay::AutoConvertCvar, this, std::placeholders::_1, std::placeholders::_2));
	
	cvarManager->registerCvar("jumpIn_pause", "0", "pauses replay", true, true, 0.0f, true, 1.0f, false);
	cvarManager->registerCvar("jumpIn_resolution", "5", "resolution how the replay is converted", true, true, 1.0f, true, 20.0f, true);
	cvarManager->registerCvar("jumpIn_limitedBoost", "1", "limits boost in JumpInReplays", true, true, 0.0f, true, 1.0f, true);
	cvarManager->registerCvar("jumpIn_showHud", "1", "shows hud in JumpInReplays", true, true, 0.0f, true, 1.0f, true);
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
				//get rid of old replay
				playerNames.clear();
				CarLayouts.clear();
				StartCars.clear();
				StartTeams.clear();
				CarPositionsPerFrame.clear();
				BallPositionPerFrame.clear();
				primeColor.clear();
				//secondColor.clear();
				BlueScoredFrame.clear();
				OrangeScoredFrame.clear();
				GameInfoPerFrame.clear();

				//save new replay
				Frame = 0;
				ReplayWrapper Replay = gameWrapper->GetGameEventAsReplay().GetReplay();
				ReplayServerWrapper ServerReplay = gameWrapper->GetGameEventAsReplay();

				ReplaySize = Replay.GetNumFrames();
				this->OneTimeSaves();
				ServerReplay.SkipToFrame(Frame);


				gameWrapper->HookEvent("Function TAGame.Replay_TA.EventPlayedFrame", std::bind(&JumpInReplay::SaveGameState, this, std::placeholders::_1));

				this->Log("Amount of Frames in Replay " + std::to_string(ReplaySize));

			}
			else {
				gameWrapper->UnhookEvent("Function TAGame.Replay_TA.EventPlayedFrame");
				gameWrapper->GetGameEventAsReplay().SetGameSpeed(1.0f);
			}
		}
		else {
			gameWrapper->UnhookEvent("Function TAGame.Replay_TA.EventPlayedFrame");
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

			if (gameWrapper->GetGameEventAsReplay().GetGameSpeed() != cvarManager->getCvar("jumpIn_resolution").getFloatValue()) {
				gameWrapper->GetGameEventAsReplay().SetGameSpeed(cvarManager->getCvar("jumpIn_resolution").getFloatValue());
			}

			//Save time and score
			SaveGameInfo();

			//BallSave
			SaveBall();

			ArrayWrapper<CarWrapper> Cars = gameWrapper->GetGameEventAsReplay().GetCars();
			//CarSave
			for (int k = 0;k < LobbySize;k++) {
				int i = 0;
				bool quit = false;
				while (playerNames.at(k) != Cars.Get(i).GetOwnerName() && quit == false) {
					//demo case
					if (i >= LobbySize - 1) {
						SaveCar0();
						quit = true;
					}
					else {
						i++;
					}
				}
				if (playerNames.at(k) == Cars.Get(i).GetOwnerName()) {
					SaveCar(i);
				}
			}
			
			Frame++;

			if (Frame >= ReplaySize) {
				gameWrapper->UnhookEvent("Function TAGame.Replay_TA.EventPlayedFrame");
				this->Log("is unhooked");
				cvarManager->getCvar("jumpIn_replaySave").setValue(false);
				if (cvarManager->getCvar("jumpIn_convert").getBoolValue() == true) {
					cvarManager->executeCommand("jumpIn_privateMatch 1");
				}
			}
		}
		else {
			gameWrapper->UnhookEvent("Function TAGame.Replay_TA.EventPlayedFrame");
			this->Log("is unhooked");
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
	CarPositionsPerFrame.push_back(CarTemp);

	//Log(std::to_string(i) + ": " + std::to_string(Car.GetReplicatedSteer()));
}

void JumpInReplay::SaveCar0() {
	CarPosition CarTemp1;
	CarTemp1.location = Vector(0, 0, 2200);
	CarTemp1.velocitiy = Vector(0, 0, 0);
	CarTemp1.rotation = Rotator(0, 0, 0);
	CarTemp1.angVelocity = Vector(0, 0, 0);
	CarTemp1.boostAmount = 0.0f;
	CarTemp1.isBoosting = false;
	//CarTemp1.hasDodge = false;
	CarTemp1.ballTouches = CarPositionsPerFrame.at(Frame - 1).ballTouches;
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

void JumpInReplay::SaveGameInfo() {
	ReplayServerWrapper Game = gameWrapper->GetGameEventAsReplay();
	GameInfo GameInfoTemp;
	GameInfoTemp.BlueScore = Game.GetTeams().Get(0).GetScore();
	GameInfoTemp.OrangeScore= Game.GetTeams().Get(1).GetScore();
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
		ReplayHZ = Replay.GetReplay().GetRecordFPS();
		ReplayFactor = 120 / ReplayHZ;
		

		LinearColor orange = gameWrapper->GetGameEventAsReplay().GetTeams().Get(1).GetPrimaryColor();
		LinearColor blue = gameWrapper->GetGameEventAsReplay().GetTeams().Get(0).GetPrimaryColor();

		//player specific saves
		for (int i = 0;i < LobbySize;i++) {
			playerNames.push_back(Replay.GetCars().Get(i).GetOwnerName());
			StartCars.push_back(Replay.GetCars().Get(i).GetbLoadoutSet());
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


//Start Private Match

void JumpInReplay::StartPrivateMatch(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (hasReplaySaved == true) {
			//opens private match
			cvarManager->executeCommand("unreal_command 'open " + Arena + "?Game=TAGame.GameInfo_Soccar_TA?Offline?Gametags=DisableGoalDelay,NoDemolish,UnlimitedTime'");
		}
		cvar.setValue(false);
	}
}


//Bot Spawning

void JumpInReplay::SpawnBots(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && hasReplaySaved == true) {
			this->Log(std::to_string(LobbySize));
			carit = 0;
			SavedFrames = Frame;
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
			//set Team color uvm
			//gameWrapper->GetGameEventAsServer().GetCars().Get(0).GetPRI().SetPlayerTeam(gameWrapper->GetGameEventAsServer().GetTeams().Get(StartTeams.at(0)));//sets team for player
			//gameWrapper->GetGameEventAsServer().GetCars().Get(0).SetCarColor(primeColor.at(0), secondColor.at(0));
			//SetTeams();
			gameWrapper->GetGameEventAsServer().GetCars().Get(carit + 1).SetHidden2(true);
			//cvarManager->executeCommand("sv_soccar_enablegoal 0", false);
			
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

void JumpInReplay::SetTeams() {
	ArrayWrapper<TeamWrapper> Teams = gameWrapper->GetGameEventAsServer().GetTeams();
	//Teams.Get(0).SetFontColor(primeColor.at(0));
	//Teams.Get(1).SetFontColor(primeColor.at(4));
}


//Controll the game

void JumpInReplay::ControllGame(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && hasReplaySaved == true) {
			//reset values
			cvarManager->getCvar("jumpIn_jumpIn").setValue(0);
			cvarManager->getCvar("jumpIn_pause").setValue(0);
			Tick = 2 * ReplayFactor;
			carit = 0;
			CountdownTime = 0;
			//isInOvertime = false;
			OrangeScore = 0;
			BlueScore = 0;
			TimeRemaining = 0;

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
	if (cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true && gameWrapper->IsInGame() == true) {
		if (Tick < ReplayFactor * (SavedFrames - 2)) {
			if (Tick % ReplayFactor == 0) {
				FrameTick = Tick / ReplayFactor;

				//cars
				for (int i = 1;i < LobbySize + 1;i++) {
					//car switching and setting cars
					if (i - 1 == carit) {
						SetCar0(i);
						//jump In
						if (cvarManager->getCvar("jumpIn_pause").getBoolValue() == false && cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() == true && GameInfoPerFrame.at(FrameTick).Countdowntime == 0) {
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
				if (gameWrapper->GetGameEventAsServer().GetPRIs().Get(0).GetBallTouches() > BallHits && cvarManager->getCvar("jumpIn_pause").getIntValue() != 1 && cvarManager->getCvar("jumpIn_jumpIn").getIntValue() == 1) {
					setBally = false;
				}
				//jumpin ghosthit exemption
				if (CarPositionsPerFrame.at(((FrameTick+2) * LobbySize) + carit).ballTouches != CarPositionsPerFrame.at(((FrameTick+1) * LobbySize) + carit).ballTouches && cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() == true) {
					setBally = false;
				}
				//score exemption
				if (Score != gameWrapper->GetGameEventAsServer().GetTotalScore()) {
					setBally = false;
				}
				if (GameInfoPerFrame.at(FrameTick).Countdowntime != CountdownTime) {
					setBally = true;
				}
				//set ball
				if (setBally == true) {
					SetBall();
				}

				//score and time
				SetGameInfo();

				//variable incrementation
				CountdownTime = GameInfoPerFrame.at(FrameTick).Countdowntime;
				//isInOvertime = gameWrapper->GetGameEventAsServer().GetbOverTime();
				Score = gameWrapper->GetGameEventAsServer().GetTotalScore();
				//TimeRemaining = gameWrapper->GetGameEventAsServer().GetSecondsRemaining();
			}
			//pause
			if (cvarManager->getCvar("jumpIn_pause").getBoolValue() == false) {
				Tick++;
			}
			else {
				if (cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() == true) {
					Tick = JumpInTick;
					setBally = true;
				}
				while (Tick % ReplayFactor != 0)
				{
					Tick--;
				}
			}
		}
		else {
			Tick = 2 * ReplayFactor;
		}
	}
	else {
		Tick = 2 * ReplayFactor;
		gameWrapper->UnhookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep");
		cvarManager->getCvar("jumpIn_controllGame").setValue(false);
	}
}

void JumpInReplay::SetCar(int i, int it) {
	CarWrapper Car = gameWrapper->GetGameEventAsServer().GetCars().Get(i);
	if (i == 0) {
		static ControllerInput input;
		input.Throttle = (CarPositionsPerFrame.at((FrameTick * LobbySize) + it).throttle - 128.0) / 128.0;
		input.Steer = (CarPositionsPerFrame.at((FrameTick * LobbySize) + it).steer - 128.0) / 128.0;
	}
	
	Car.SetLocation(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).location);
	Car.SetVelocity(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).velocitiy);
	Car.SetRotation(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).rotation);
	Car.SetAngularVelocity(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).angVelocity, false);
	Car.SetbOverrideHandbrakeOn(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).handbrake);
	Car.GetBoostComponent().SetBoostAmount(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).boostAmount);
	Car.ForceBoost(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).isBoosting);
	//Car.SetbCanJump(CarPositionsPerFrame.at((FrameTick * LobbySize) + it).hasDodge);

	//Log(std::to_string(i)+": "+std::to_string(CarPositionsPerFrame.at(((FrameTick) * LobbySize) + it).location.X) + ", " + std::to_string(CarPositionsPerFrame.at(((FrameTick) * LobbySize) + it).location.Y) + ", " + std::to_string(CarPositionsPerFrame.at(((FrameTick) * LobbySize) + it).location.Z));
}

void JumpInReplay::SetCar0(int i) {
	CarWrapper Car = gameWrapper->GetGameEventAsServer().GetCars().Get(i);

	Car.SetLocation(Vector(0, 0, 5000));
	Car.SetVelocity(Vector(0, 0, 0));
	Car.SetRotation(Rotator(0, 0, 0));
	Car.SetAngularVelocity(Vector(0, 0, 0), false);
	//Car.GetBoostComponent().SetBoostAmount();
	Car.ForceBoost(0);
	//Car.SetbCanJump(0);
}

void JumpInReplay::SetBall() {
	BallWrapper Ball = gameWrapper->GetGameEventAsServer().GetBall();
	Ball.SetLocation(BallPositionPerFrame.at(FrameTick).location);
	Ball.SetVelocity(BallPositionPerFrame.at(FrameTick).velocitiy);
	Ball.SetRotation(BallPositionPerFrame.at(FrameTick).rotation);
	Ball.SetAngularVelocity(BallPositionPerFrame.at(FrameTick).angVelocity, false);
}

void JumpInReplay::SetGameInfo() {
	gameWrapper->GetGameEventAsServer().SetScoreAndTime(gameWrapper->GetGameEventAsServer().GetLocalPrimaryPlayer(), GameInfoPerFrame.at(FrameTick).BlueScore, GameInfoPerFrame.at(FrameTick).OrangeScore, GameInfoPerFrame.at(FrameTick).TimeRemaining, false, false);

	if (GameInfoPerFrame.at(FrameTick).Countdowntime != CountdownTime) {
		if (GameInfoPerFrame.at(FrameTick).Countdowntime == 0) {
			gameWrapper->GetGameEventAsServer().BroadcastGoMessage();
		}
		else {
			gameWrapper->GetGameEventAsServer().BroadcastCountdownMessage(GameInfoPerFrame.at(FrameTick).Countdowntime);
		}
	}
}


//skip

void JumpInReplay::Skip(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getIntValue() != 0) {
		if (gameWrapper->IsInGame() == true && cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true) {
			setBally = true;
			if (Tick < 150 && Tick >= 2 * ReplayFactor) { Tick = (ReplayFactor * SavedFrames) + 120 * cvar.getFloatValue(); }
			else {
				Tick = Tick + (120 * cvar.getFloatValue());
			}
			if (Tick < 2 * ReplayFactor) { Tick = 2 * ReplayFactor; }
			if (Tick > ReplayFactor * SavedFrames) { Tick = 2 * ReplayFactor; }
		}
		cvar.setValue(0);
	}
}


//car switching

void JumpInReplay::SwitchCars(std::string oldValue, CVarWrapper cvar) {
	if (cvar.getBoolValue() == true) {
		if (gameWrapper->IsInGame() == true && cvarManager->getCvar("jumpIn_controllGame").getBoolValue() == true) {
			ServerWrapper Game = gameWrapper->GetGameEventAsServer();
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
	}
	else {
		gameWrapper->UnhookEvent("Function TAGame.GameInfo_Replay_TA.InitGame");
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
		cvarManager->executeCommand("bind XboxTypeS_DPad_Left 'default_left; jumpIn_skip -10'");
		cvarManager->executeCommand("bind XboxTypeS_DPad_Up 'default_up; jumpIn_switchPlayer 1'");
		cvarManager->executeCommand("bind XboxTypeS_DPad_Down 'default_down; jumpIn_switchBack 1'");
		cvarManager->executeCommand("bind XboxTypeS_Back 'toggle jumpIn_jumpIn 1 0'");
		cvarManager->executeCommand("bind XboxTypeS_LeftThumbStick 'toggle jumpIn_pause 1 0'");
		//keyboard
		cvarManager->executeCommand("bind Right 'jumpIn_skip 10'");
		cvarManager->executeCommand("bind Left 'jumpIn_skip -10'");
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
			DrawTime(canvas);
			DrawPause(canvas);
			DrawTimeline(canvas);
			DrawTimelineSpent(canvas);
			DrawPlayer(canvas);
			DrawJumpIn(canvas);
			DrawBlueScored(canvas);
			DrawOrangeScored(canvas);
		}
	}
}


//time

void JumpInReplay::DrawTime(CanvasWrapper canvas) {
	//calculate minutes and seconds
	MinutesAvailable = SavedFrames / (60 * ReplayHZ);
	SecondsAvailable = (SavedFrames / ReplayHZ) - (MinutesAvailable * 60);
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
	canvas.SetPosition(Vector2{ 1361,1000 });
	canvas.DrawString(std::to_string(MinutesDone) + ":" + StringSecondsDone + "/" + std::to_string(MinutesAvailable) + ":" + StringSecondsAvailable, 1.5, 1.5, true, false);
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
		canvas.FillTriangle(Vector2{ 460,1000 }, Vector2{ 460,1024 }, Vector2{ 484,1012 }, color);
	}
	else {
		//pause button
		canvas.SetColor(color);
		canvas.DrawLine(Vector2{ 460,1000 }, Vector2{ 468,1000 }, 24);
		canvas.DrawLine(Vector2{ 476,1000 }, Vector2{ 484,1000 }, 24);
	}
}


//Timeline

void JumpInReplay::DrawTimeline(CanvasWrapper canvas) {
	canvas.SetColor(232, 232, 232, 150);
	canvas.SetPosition(Vector2{ 460,960 });
	canvas.DrawLine(Vector2{ 460,960 }, Vector2{ 1460,960 }, 6);
}

void JumpInReplay::DrawTimelineSpent(CanvasWrapper canvas) {
	int length = 0;
	float ReplaySpent = FrameTick;
	float ReplayLength = SavedFrames;
	length = 1000 * (ReplaySpent / ReplayLength);
	canvas.SetColor(0, 126, 255, 230);
	canvas.SetPosition(Vector2{ 460,975 });
	canvas.DrawLine(Vector2{ 460,960 }, Vector2{ (460 + length),960 }, 6);
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
	canvas.SetPosition(Vector2{ 650,920 });
	canvas.DrawString(playerNames.at(carit), 1.5, 1.5, true, false);

}


//JumpIn

void JumpInReplay::DrawJumpIn(CanvasWrapper canvas) {
	canvas.SetPosition(Vector2{ 1240,920 });
	if (cvarManager->getCvar("jumpIn_jumpIn").getBoolValue() != true) {
		//jumpout
		canvas.SetColor(114, 225, 78, 255);
		canvas.DrawString("JumpIn", 1.5, 1.5, true, false);
	}
	else {
		//jumpIn
		canvas.SetColor(255, 1, 1, 255);
		canvas.DrawString("JumpOut", 1.5, 1.5, true, false);
	}
}


//show goals in Timelin

void JumpInReplay::DrawBlueScored(CanvasWrapper canvas) {
	LinearColor color = { 12, 136, 252, 255 };
	for (int i = 0;i < BlueScoredFrame.size();i++) {
		float ReplayLength = SavedFrames;
		float FrameScored = BlueScoredFrame.at(i);
		int position = 460 + (1000 * ( FrameScored / ReplayLength));
		canvas.FillTriangle(Vector2{ position,963 }, Vector2{ position-3,972 }, Vector2{ position+3,972 }, color);
	}
}

void JumpInReplay::DrawOrangeScored(CanvasWrapper canvas) {
	LinearColor color = { 252, 124, 12, 255 };
	for (int i = 0;i < OrangeScoredFrame.size();i++) {
		float ReplayLength = SavedFrames;
		float FrameScored = OrangeScoredFrame.at(i);
		int position = 460 + (1000 * (FrameScored / ReplayLength));
		canvas.FillTriangle(Vector2{ position,963 }, Vector2{ position - 3,972 }, Vector2{ position + 3,972 }, color);
	}
}


//Bakkesmod Log

void JumpInReplay::Log(std::string msg) {
	cvarManager->log(msg);

	return;
}