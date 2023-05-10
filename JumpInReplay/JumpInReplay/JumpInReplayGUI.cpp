#include "pch.h"
#include "JumpInReplay.h"
#include "JumpInFile.h"
#include "compatibility.h"

//Plugin Settings Window code here
/*std::string JumpInReplay::GetPluginName() {
	return "JumpInReplay";
}
*/
// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> JumpInReplay
/*void JumpInReplay::RenderSettings() {
	if (ImGui::Button("Convert Replay")) { cvarManager->executeCommand("jumpIn_convert 1"); }
	if (ImGui::Button("Open Saved Replay")) { cvarManager->executeCommand("jumpIn_openReplay 1"); }
	ImGui::Text("do not change Teams, restart Match or go to Spectate");
	ImGui::Separator();
	ImGui::Text("Settings:");
	ImGui::Separator();
	bool settingAutoConvert = cvarManager->getCvar("jumpIn_autoConvert").getBoolValue();
	if(ImGui::Checkbox("Automatically Convert Replay",&settingAutoConvert)){ cvarManager->getCvar("jumpIn_autoConvert").setValue(settingAutoConvert); }
	float settingResolution= cvarManager->getCvar("jumpIn_resolution").getFloatValue();
	if(ImGui::SliderFloat("Conversion Resolution",&settingResolution,0.01,20.0)){ cvarManager->getCvar("jumpIn_resolution").setValue(settingResolution); }
	bool settingLimitedBoost = cvarManager->getCvar("jumpIn_limitedBoost").getBoolValue();
	if (ImGui::Checkbox("Limited Boost in Replay", &settingLimitedBoost)) { cvarManager->getCvar("jumpIn_limitedBoost").setValue(settingLimitedBoost); }
	bool settingNormalTraining = cvarManager->getCvar("jumpIn_inputToUnpause").getBoolValue();
	if (ImGui::Checkbox("Normal Training Start", &settingNormalTraining)) { cvarManager->getCvar("jumpIn_inputToUnpause").setValue(settingNormalTraining); }
	bool settingHUD = cvarManager->getCvar("jumpIn_showHud").getBoolValue();
	if (ImGui::Checkbox("Show HUD in JumpInReplay", &settingHUD)) { cvarManager->getCvar("jumpIn_showHud").setValue(settingHUD); }
	ImGui::Text("");
	bool settingJustKeyframes = cvarManager->getCvar("jumpIn_convertKeyframes").getBoolValue();
	if (ImGui::Checkbox("Just Convert Keyframes", &settingJustKeyframes)) { cvarManager->getCvar("jumpIn_convertKeyframes").setValue(settingJustKeyframes); }
	float settingBeforeKeyframe = cvarManager->getCvar("jumpIn_timeBeforKeyframe").getFloatValue();
	if (ImGui::SliderFloat("Time that is Converted Before a Keyframe", &settingBeforeKeyframe, 0.0, 120.0)) { cvarManager->getCvar("jumpIn_timeBeforKeyframe").setValue(settingBeforeKeyframe); }
	float settingAfterKeyframe = cvarManager->getCvar("jumpIn_timeAfterKeyframe").getFloatValue();
	if (ImGui::SliderFloat("Time that is Converted After a Keyframe", &settingAfterKeyframe, 0.0, 60.0)) { cvarManager->getCvar("jumpIn_timeAfterKeyframe").setValue(settingAfterKeyframe); }
	ImGui::Separator();
	ImGui::Text("Bindings:");
	ImGui::Separator();
	if (ImGui::Button("apply Standard Bindings")) { cvarManager->getCvar("jumpIn_bindings").setValue(true); }
	ImGui::Text("");
	ImGui::Text("Standard bindings are:");
	ImGui::Text("DPadRight/RightArrow: skips 10s forward in Replay");
	ImGui::Text("DPadLeft/LeftArrow: skips 5s back in Replay");
	ImGui::Text("DPadUp/UpArrow: selects spectated Player");
	ImGui::Text("DPadDown/DownArrow: selects previously spectated Player");
	ImGui::Text("LeftStickPress/B: pauses Replay (also resets shot)");
	ImGui::Text("Back/Select/V: toggles JumpInMode (you have to unpause)");
	ImGui::Text("");
	ImGui::Text("if you want to change the bindings you can do it in the bakkesmod bindings tab");
	ImGui::Separator();
	ImGui::Text("plugin made by: Atomus#5492");
	std::string settingPluginVersion = "plugin Version: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_PATCH) + "." + std::to_string(VERSION_BUILD);
	ImGui::Text(settingPluginVersion.c_str());
}*/



// Do ImGui rendering here
void JumpInReplay::Render()
{
	ImGui::SetNextWindowSize(ImVec2(750, 100));
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	//Compatibility compatibility;
	if (compatibility.needsGUI) {	
		compatibility.ImGuiForAsk();
	}
	
	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string JumpInReplay::GetMenuName()
{
	return "JumpInReplay";
}

// Title to give the menu
std::string JumpInReplay::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void JumpInReplay::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool JumpInReplay::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool JumpInReplay::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void JumpInReplay::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void JumpInReplay::OnClose()
{
	isWindowOpen_ = false;
}

