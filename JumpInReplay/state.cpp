#pragma once

#include "pch.h"
#include "JumpInReplay.h"
#include "state.h"

ActorState::ActorState() {
	location = Vector(0, 0, 0);
	velocity = Vector(0, 0, 0);
	rotation = Rotator(0, 0, 0);
	angVelocity = Vector(0, 0, 0);
}
ActorState::ActorState(ActorWrapper a) {
	location = a.GetLocation();
	velocity = a.GetVelocity();
	rotation = a.GetRotation();
	angVelocity = a.GetAngularVelocity();
}

void ActorState::apply(ActorWrapper a) const {
	a.SetLocation(location);
	a.SetVelocity(velocity);
	a.SetRotation(rotation);
	a.SetAngularVelocity(angVelocity, false);
}
CarState::CarState() {
	actorState = ActorState();
	boostAmount = 0;
	hasDodge = false;
	lastJumped = 0;
}
CarState::CarState(CarWrapper c) {
	actorState = ActorState(c);
	boostAmount = c.GetBoostComponent().IsNull() ? 0 : c.GetBoostComponent().GetCurrentBoostAmount();
	// Save last jump time only if the player jumped.
	// After applying this, we will remove the player's dodge when the jump timer expires.
	lastJumped = !c.GetbJumped() || c.GetJumpComponent().IsNull() ? -1 : c.GetJumpComponent().GetInactiveTime();
	hasDodge = !c.GetbDoubleJumped() && lastJumped < MAX_DODGE_TIME;
}

void CarState::apply(CarWrapper c) const {
	actorState.apply(c);
	if (!c.GetBoostComponent().IsNull()) {
		c.GetBoostComponent().SetCurrentBoostAmount(boostAmount);
	}
	c.SetbDoubleJumped(!hasDodge);
	c.SetbJumped(!hasDodge);
}

GameState::GameState() {
	ball = ActorState();
	car = CarState();
}

GameState::GameState(ServerWrapper sw) {
	ball = ActorState(sw.GetBall());
	car = CarState(sw.GetGameCar());
}

GameState::GameState(ArrayWrapper<CarWrapper> acw, BallWrapper bw) {
	ball = ActorState(bw);
	
	for (int i = 0;i < acw.Count();i++) {
		cars[i] = CarState(acw.Get(i));
	}
}

void GameState::apply(ServerWrapper sw) const {
	if (sw.GetBall().IsNull() || sw.GetGameCar().IsNull()) {
		return;
	}
	ball.apply(sw.GetBall());
	for (int i = 0;i < sw.GetCars().Count();i++) {
		cars[i].apply(sw.GetCars().Get(i));
	}
	car.apply(sw.GetGameCar());
}

void Playtest::Run()
{
	auto server = gameWrapper->GetCurrentGameState();

	CarWrapper player_car_wrapper = server.GetLocalPrimaryPlayer().GetCar();

	// Remove current cars
	auto cars = server.GetCars();
	for (int i = cars.Count() - 1; i >= 0; i--) {
		CarWrapper car = cars.Get(i);
		if (car.memory_address != player_car_wrapper.memory_address) {
			AIControllerWrapper controller_wrapper = car.GetAIController();
			if (controller_wrapper) {
				server.RemovePlayer(controller_wrapper);
			}
			server.RemoveCar(car);
		}
	}

	server.SetbRandomizedBotLoadouts(false);

	server.SpawnBot(23, "Bob");
	LOG("SpawBot: Bob");
	server.SpawnBot(403, "Joe");
	LOG("SpawBot: Joe");

	for (int i = 0; i < server.GetCars().Count(); i++) {
		CarWrapper car = server.GetCars().Get(i);
		PriWrapper pri = car.GetPRI();
		if (pri.GetbBot()) {
			BotLoadoutData loadout;
			loadout.team = 255;
			loadout.custom_color_id = 6;
			loadout.custom_finish_id = 268;
			loadout.team_finish_id = 268;

			gameWrapper->SetBotLoadout(pri, loadout);
		}
	}
}
