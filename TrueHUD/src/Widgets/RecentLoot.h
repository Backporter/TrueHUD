#pragma once
#include "TrueHUDAPI.h"

namespace Scaleform
{
	class RecentLoot : public TRUEHUD_API::WidgetBase
	{
	public:
		RecentLoot(ConsoleRE::GPtr<ConsoleRE::GFxMovieView> a_view) :
			WidgetBase(a_view)
		{}

		virtual void Update(float a_deltaTime) override;
		virtual void Initialize() override;
		virtual void Dispose() override;

		void AddMessage(ConsoleRE::TESBoundObject* a_object, const char* a_name, uint32_t a_count);

	protected:
		virtual void UpdatePosition();
		virtual void LoadConfig();

		// The process icon code was adapted, mostly without changes, from SkyUI's actionscript functions.
		void ProcessArmorIcon(ConsoleRE::TESBoundObject* a_object, std::string& a_outIconLabel, uint32_t& a_outIconColor);
		void ProcessBookIcon(ConsoleRE::TESBoundObject* a_object, std::string& a_outIconLabel, uint32_t& a_outIconColor);
		void ProcessMiscIcon(ConsoleRE::TESBoundObject* a_object, std::string& a_outIconLabel, uint32_t& a_outIconColor);
		void ProcessWeaponIcon(ConsoleRE::TESBoundObject* a_object, std::string& a_outIconLabel, uint32_t& a_outIconColor);
		void ProcessAmmoIcon(ConsoleRE::TESBoundObject* a_object, std::string& a_outIconLabel, uint32_t& a_outIconColor);
		void ProcessPotionIcon(ConsoleRE::TESBoundObject* a_object, std::string& a_outIconLabel, uint32_t& a_outIconColor);
		void ProcessSoulGemIcon(ConsoleRE::TESBoundObject* a_object, std::string& a_outIconLabel, uint32_t& a_outIconColor);
	};
}
