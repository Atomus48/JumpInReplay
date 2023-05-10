#pragma once


class Compatibility//: public JumpInReplay
{
public:

	bool CheckForIncompatibility();
	void DisableIncompatiblePlugin(std::string plugin);
	void AskForDisable(std::string plugin);
	void ImGuiForAsk();

	//std::vector<std::string> LoadedPlugins= gameWrapper->GetPluginManager().GetLoadedPlugins()
	std::vector<std::string> IncompatiblePlugin = { "trueballheightindicator"};
	std::vector<std::string> LoadedPlugins;
	std::string currentNotCompatiblePlugin= "___";
	std::shared_ptr<GameWrapper> CompGameWrapper;


	bool needsGUI = false;
};

