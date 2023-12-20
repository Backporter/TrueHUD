#include "Scaleform/Scaleform.h"

#include "Scaleform/TrueHUDMenu.h"
#include "Offsets.h"

namespace Scaleform
{
	void Register()
	{
		TrueHUDMenu::Register();

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered all movies");
	}
}

