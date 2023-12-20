#include "Papyrus.h"

#include "Settings.h"

namespace Papyrus
{
	void TrueHUD_MCM::OnConfigClose(ConsoleRE::TESQuest*)
	{
		Settings::ReadSettings();
	}

	bool TrueHUD_MCM::Register(ConsoleRE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->BindNativeMethod(new ConsoleRE::BSScript::NativeFunction<false, decltype(OnConfigClose), ConsoleRE::TESQuest, void>("OnConfigClose", "TrueHUD_MCM", OnConfigClose));

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered TrueHUD_MCM class");
		return true;
	}

	void Register()
	{
		auto papyrus = API::GetPapyrusInterface();
		papyrus->RegisterPapyrusFunctions(TrueHUD_MCM::Register);
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered papyrus functions");
	}
}
