#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

class ActorState {
public:
	Vector location;
	Vector velocity;
	Rotator rotation;
	Vector angVelocity;

	ActorState();
	ActorState(ActorWrapper a);

	void apply(ActorWrapper a) const;
};

class CarState {
public:
	ActorState actorState;
	float boostAmount;
	bool hasDodge;
	float lastJumped; // cannot apply; used to reset dodge in record().

	CarState();
	CarState(CarWrapper c);

	void apply(CarWrapper c) const;
};

class GameState {
public:
	ActorState ball;
	CarState car;
	std::vector<CarState> cars;

	GameState();
	GameState(ServerWrapper sw);
	GameState(ArrayWrapper<CarWrapper> acw, BallWrapper bw);

	void apply(ServerWrapper sw) const;

};

