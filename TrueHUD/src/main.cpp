#include "../../../OrbisUtil/include/RelocationManager.h"
#include "../../../OrbisUtil/include/Logger.h"
#include "../../../OrbisUtil/include/FileSystem.h"

#include "Hooks.h"
#include "Papyrus.h"
#include "Settings.h"
#include "HUDHandler.h"
#include "Scaleform/Scaleform.h"

extern "C" __declspec (dllexport) int module_start(size_t argc, const void* argv) { xUtilty::RelocationManager(); return 0; }
extern "C" __declspec (dllexport) int module_stop(size_t argc, const void* argv) { return 0; }

void MessageHandler(Interface::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) 
	{
	case Interface::MessagingInterface::kDataLoad:
		Scaleform::Register();
		HUDHandler::Register();
		Settings::Initialize();
		Settings::ReadSettings();		
		HUDHandler::GetSingleton()->Initialize();
		break;
	case Interface::MessagingInterface::kLoading:
		HUDHandler::GetSingleton()->OnPreLoadGame();
		break;
	case Interface::MessagingInterface::kLoaded:
	case Interface::MessagingInterface::kNewGame:
		Settings::OnPostLoadGame();
		break;
	}
}

namespace
{
	void InitializeLog()
	{
		//
		xUtilty::Log::GetSingleton(0)->OpenRelitive(xUtilty::FileSystem::Download, "OSEL/Plugins/TrueHUD/TrueHUD.log");
	}
}

extern "C" DLLEXPORT bool Query(const Interface::QueryInterface* a_skse, PluginInfo* a_info)
{
	a_info->SetPluginName("TrueHUD");
	a_info->SetPluginVersion(1);
	return true;
}

extern "C" DLLEXPORT bool Load(const Interface::QueryInterface* a_skse)
{
	InitializeLog();

	API::Init(a_skse);
	API::AllocTrampoline(1 << 8);

	auto messaging = API::GetMessagingInterface();
	if (!messaging->RegisterPluginCallback("SELF", MessageHandler)) 
	{
		return false;
	}

	Hooks::Install();
	Papyrus::Register();

	return true;
}

extern "C" DLLEXPORT bool Revert() { return true; }

extern "C" DLLEXPORT void* RequestPluginAPI(const TRUEHUD_API::InterfaceVersion a_interfaceVersion)
{
	auto api = Messaging::TrueHUDInterface::GetSingleton();

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "TrueHUD::RequestPluginAPI called, InterfaceVersion %d", a_interfaceVersion);

	switch (a_interfaceVersion) {
	case TRUEHUD_API::InterfaceVersion::V1:
		[[fallthrough]];
	case TRUEHUD_API::InterfaceVersion::V2:
		[[fallthrough]];
	case TRUEHUD_API::InterfaceVersion::V3:
		[[fallthrough]];
	case TRUEHUD_API::InterfaceVersion::V4:
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "TrueHUD::RequestPluginAPI returned the API singleton");
		return static_cast<void*>(api);
	}

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "TrueHUD::RequestPluginAPI requested the wrong interface version");
	return nullptr;
}
