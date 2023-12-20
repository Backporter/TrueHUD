#pragma once

namespace Papyrus
{
	class TrueHUD_MCM
	{
	public:
		static void OnConfigClose(ConsoleRE::TESQuest*);

		static bool Register(ConsoleRE::BSScript::IVirtualMachine* a_vm);
	};

	void Register();
}
