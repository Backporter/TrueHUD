#include "Settings.h"

#include "HUDHandler.h"
#include "ModAPI.h"

void Settings::Initialize()
{
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Initializing...");

	//using DefaultObject = ConsoleRE::BGSDefaultObjectManager::DefaultObject;
	//auto defaultObjectManager = ConsoleRE::BGSDefaultObjectManager::GetSingleton();
	//if (defaultObjectManager) {
	//	glob_survivalModeEnabled = defaultObjectManager->GetObject<ConsoleRE::TESGlobal>(DefaultObject::kSurvivalModeEnabled);
	//	glob_survivalHealthPenaltyPercent = defaultObjectManager->GetObject<ConsoleRE::TESGlobal>(DefaultObject::kSurvivalColdPenalty);
	//	glob_survivalMagickaPenaltyPercent = defaultObjectManager->GetObject<ConsoleRE::TESGlobal>(DefaultObject::kSurvivalSleepPenalty);
	//	glob_survivalStaminaPenaltyPercent = defaultObjectManager->GetObject<ConsoleRE::TESGlobal>(DefaultObject::kSurvivalHungerPenalty);
	//	//kywd_Dragon = defaultObjectManager->GetObject<ConsoleRE::BGSKeyword>(DefaultObject::kKeywordDragon);
	//}	

	auto dataHandler = ConsoleRE::TESDataHandler::GetSingleton();
	if (dataHandler) 
	{
		kywd_noSoulTrap						= dataHandler->LookupForm<ConsoleRE::BGSKeyword>(0x103AD2, "Skyrim.esm");
		kywd_Dragon							= dataHandler->LookupForm<ConsoleRE::BGSKeyword>(0x35D59, "Skyrim.esm");
		
		glob_survivalModeEnabled			= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x314A, "Update.esm");
		glob_survivalHealthPenaltyPercent	= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x2EDE, "Update.esm");
		glob_survivalMagickaPenaltyPercent	= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x2EE0, "Update.esm");
		glob_survivalStaminaPenaltyPercent	= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x2EDF, "Update.esm");
		
		glob_trueHUDVersion					= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x801, "TrueHUD.esl");
		glob_trueHUDSpecialResourceBars		= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x802, "TrueHUD.esl");
	}

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");
}

void Settings::ReadSettings()
{
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Reading boss .inis...");

	constexpr auto tdm_path = "/app0/data/SKSE/Plugins/TrueDirectionalMovement/";
	constexpr auto truehud_path = "/app0/data/SKSE/Plugins/TrueHUD/";
	constexpr auto ext = ".ini";

	constexpr auto tdm_baseini = "/app0/data/SKSE/Plugins/TrueDirectionalMovement/TrueDirectionalMovement_base.ini";
	constexpr auto truehud_baseini = "/app0/data/SKSE/Plugins/TrueHUD/TrueHUD_base.ini";

	constexpr auto defaultSettingsPath = "/app0/data/MCM/Config/TrueHUD/settings.ini";
	constexpr auto mcmPath = "/app0/data/MCM/Settings/TrueHUD.ini";

	const auto dataHandler = ConsoleRE::TESDataHandler::GetSingleton();

	const auto readBossIni = [&](const char* path) 
	{
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "  Reading %s...", path);
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetMultiKey();

		ini.LoadFile(path);

		CSimpleIniA::TNamesDepend races;
		ini.GetAllValues("BossRecognition", "Race", races);
		CSimpleIniA::TNamesDepend removedRaces;
		ini.GetAllValues("BossRecognition", "RemoveRace", removedRaces);

		CSimpleIniA::TNamesDepend locRefTypes;
		ini.GetAllValues("BossRecognition", "LocRefType", locRefTypes);
		CSimpleIniA::TNamesDepend removedLocRefTypes;
		ini.GetAllValues("BossRecognition", "RemoveLocRefType", removedLocRefTypes);

		CSimpleIniA::TNamesDepend npcs;
		ini.GetAllValues("BossRecognition", "NPC", npcs);
		CSimpleIniA::TNamesDepend removedNpcs;
		ini.GetAllValues("BossRecognition", "RemoveNPC", removedNpcs);

		CSimpleIniA::TNamesDepend blacklistedNPCs;
		ini.GetAllValues("BossRecognition", "NPCBlacklist", blacklistedNPCs);
		CSimpleIniA::TNamesDepend removedBlacklistedNPCs;
		ini.GetAllValues("BossRecognition", "RemoveNPCBlacklist", removedBlacklistedNPCs);

		for (const auto& entry : races) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			uint32_t formID;
			formID = std::stoi(formIDstr.data(), 0, 16);
			auto race = dataHandler->LookupForm<ConsoleRE::TESRace>(formID, modName.c_str());
			if (race) {
				bossRaces.emplace(race);
			}
		}

		for (const auto& entry : removedRaces) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			uint32_t formID;
			formID = std::stoi(formIDstr.data(), 0, 16);
			auto race = dataHandler->LookupForm<ConsoleRE::TESRace>(formID, modName.c_str());
			if (race) {
				bossRaces.erase(race);
			}
		}

		for (const auto& entry : locRefTypes) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			if (formIDstr.empty()) {
				continue;
			}
			uint32_t formID = std::stoi(formIDstr.data(), 0, 16);
			auto locRefType = dataHandler->LookupForm<ConsoleRE::BGSLocationRefType>(formID, modName.c_str());
			if (locRefType) {
				bossLocRefTypes.emplace(locRefType);
			}
		}

		for (const auto& entry : removedLocRefTypes) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			if (formIDstr.empty()) {
				continue;
			}
			uint32_t formID = std::stoi(formIDstr.data(), 0, 16);
			auto locRefType = dataHandler->LookupForm<ConsoleRE::BGSLocationRefType>(formID, modName.c_str());
			if (locRefType) {
				bossLocRefTypes.erase(locRefType);
			}
		}

		for (const auto& entry : npcs) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			if (formIDstr.empty()) {
				continue;
			}
			uint32_t formID = std::stoi(formIDstr.data(), 0, 16);
			auto npc = dataHandler->LookupForm<ConsoleRE::TESNPC>(formID, modName.c_str());
			if (npc) {
				bossNPCs.emplace(npc);
			}
		}

		for (const auto& entry : removedNpcs) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			if (formIDstr.empty()) {
				continue;
			}
			uint32_t formID = std::stoi(formIDstr.data(), 0, 16);
			auto npc = dataHandler->LookupForm<ConsoleRE::TESNPC>(formID, modName.c_str());
			if (npc) {
				bossNPCs.erase(npc);
			}
		}

		for (const auto& entry : blacklistedNPCs) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			if (formIDstr.empty()) {
				continue;
			}
			uint32_t formID = std::stoi(formIDstr.data(), 0, 16);
			auto npc = dataHandler->LookupForm<ConsoleRE::TESNPC>(formID, modName.c_str());
			if (npc) {
				bossNPCBlacklist.emplace(npc);
			}
		}

		for (const auto& entry : removedBlacklistedNPCs) {
			std::string str = entry.pItem;
			auto split = str.find(':');
			auto modName = str.substr(0, split);
			auto formIDstr = str.substr(split + 1, str.find(' ', split + 1) - (split + 1));
			if (formIDstr.empty()) {
				continue;
			}
			uint32_t formID = std::stoi(formIDstr.data(), 0, 16);
			auto npc = dataHandler->LookupForm<ConsoleRE::TESNPC>(formID, modName.c_str());
			if (npc) {
				bossNPCBlacklist.erase(npc);
			}
		}
	};

	readBossIni(tdm_baseini);

	/*
	if (std::filesystem::is_directory(tdm_path)) 
	{
		for (const auto& file : std::filesystem::directory_iterator(tdm_path))  // read all ini files in Data/SKSE/Plugins/TrueDirectionalMovement folder first, for backwards compatibility
		{
			if (std::filesystem::is_regular_file(file) && file.path().extension() == ext) 
			{
				auto path = file.path();
				if (path != tdm_baseini) 
				{
					readBossIni(path);
				}
			}
		}
	}
	*/

	readBossIni(truehud_baseini);

	/*
	if (std::filesystem::is_directory(truehud_path)) 
	{
		for (const auto& file : std::filesystem::directory_iterator(truehud_path))  // read all ini files in Data/SKSE/Plugins/TrueHUD folder
		{
			if (std::filesystem::is_regular_file(file) && file.path().extension() == ext) 
			{
				auto path = file.path();
				if (path != truehud_baseini) 
				{
					readBossIni(path);
				}
			}
		}
	}
	*/

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");

	const auto readMCM = [&](const char* path) 
	{
		CSimpleIniA mcm;
		mcm.SetUnicode();
		mcm.LoadFile(path);

		// General
		ReadBoolSetting(mcm, "General", "bEnableActorInfoBars", bEnableActorInfoBars);
		ReadBoolSetting(mcm, "General", "bEnableBossBars", bEnableBossBars);
		ReadBoolSetting(mcm, "General", "bEnablePlayerWidget", bEnablePlayerWidget);
		ReadBoolSetting(mcm, "General", "bEnableRecentLoot", bEnableRecentLoot);
		ReadBoolSetting(mcm, "General", "bEnableFloatingText", bEnableFloatingText);
		ReadBoolSetting(mcm, "General", "bHideVanillaTargetBar", bHideVanillaTargetBar);

		// Info Bar Widget
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDirection", (uint32_t&)uInfoBarDirection);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDisplayHostiles", (uint32_t&)uInfoBarDisplayHostiles);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDisplayTeammates", (uint32_t&)uInfoBarDisplayTeammates);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDisplayOthers", (uint32_t&)uInfoBarDisplayOthers);
		ReadBoolSetting(mcm, "ActorInfoBars", "bInfoBarDisplayPhantomBars", bInfoBarDisplayPhantomBars);
		ReadBoolSetting(mcm, "ActorInfoBars", "bInfoBarDisplaySpecialPhantomBar", bInfoBarDisplaySpecialPhantomBar);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDisplayName", (uint32_t&)uInfoBarDisplayName);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDisplayDamageCounter", (uint32_t&)uInfoBarDisplayDamageCounter);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDisplayIndicator", (uint32_t&)uInfoBarDisplayIndicator);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarDamageCounterAlignment", (uint32_t&)uInfoBarDamageCounterAlignment);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarResourcesModeTarget", (uint32_t&)uInfoBarResourcesModeTarget);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarResourcesModeHostiles", (uint32_t&)uInfoBarResourcesModeHostiles);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarResourcesModeTeammates", (uint32_t&)uInfoBarResourcesModeTeammates);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarResourcesModeOthers", (uint32_t&)uInfoBarResourcesModeOthers);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarSpecialModeTarget", (uint32_t&)uInfoBarSpecialModeTarget);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarSpecialModeHostiles", (uint32_t&)uInfoBarSpecialModeHostiles);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarSpecialModeTeammates", (uint32_t&)uInfoBarSpecialModeTeammates);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarSpecialModeOthers", (uint32_t&)uInfoBarSpecialModeOthers);
		ReadBoolSetting(mcm, "ActorInfoBars", "bInfoBarScaleWithDistance", bInfoBarScaleWithDistance);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarAnchor", (uint32_t&)uInfoBarAnchor);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarOffsetZ", fInfoBarOffsetZ);
		ReadBoolSetting(mcm, "ActorInfoBars", "bInfoBarLevelColorOutline", bInfoBarLevelColorOutline);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarLevelThreshold", (uint32_t&)uInfoBarLevelThreshold);
		ReadUInt32Setting(mcm, "ActorInfoBars", "uInfoBarIndicatorMode", (uint32_t&)uInfoBarIndicatorMode);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarPhantomDuration", fInfoBarPhantomDuration);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarDamageCounterDuration", fInfoBarDamageCounterDuration);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarScale", fInfoBarScale);
		ReadBoolSetting(mcm, "ActorInfoBars", "bInfoBarUseHUDOpacity", bInfoBarUseHUDOpacity);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarOpacity", fInfoBarOpacity);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarWidth", fInfoBarWidth);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarResourceWidth", fInfoBarResourceWidth);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarTargetWidth", fInfoBarTargetWidth);
		ReadFloatSetting(mcm, "ActorInfoBars", "fInfoBarTargetResourceWidth", fInfoBarTargetResourceWidth);

		// Boss Info Bar Widget
		ReadUInt32Setting(mcm, "BossBars", "uBossBarDirection", (uint32_t&)uBossBarDirection);
		ReadBoolSetting(mcm, "BossBars", "bBossBarDisplayPhantomBars", bBossBarDisplayPhantomBars);
		ReadBoolSetting(mcm, "BossBars", "bBossBarDisplaySpecialPhantomBar", bBossBarDisplaySpecialPhantomBar);
		ReadBoolSetting(mcm, "BossBars", "bBossBarDisplayDamageCounter", bBossBarDisplayDamageCounter);
		ReadBoolSetting(mcm, "BossBars", "bBossBarDisplayIndicator", bBossBarDisplayIndicator);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarDamageCounterAlignment", (uint32_t&)uBossBarDamageCounterAlignment);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarResourcesMode", (uint32_t&)uBossBarResourcesMode);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarSpecialMode", (uint32_t&)uBossBarSpecialMode);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarResourceAlignment", (uint32_t&)uBossBarResourceAlignment);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarNameAlignment", (uint32_t&)uBossBarNameAlignment);
		ReadBoolSetting(mcm, "BossBars", "bBossBarLevelColorOutline", bBossBarLevelColorOutline);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarLevelThreshold", (uint32_t&)uBossBarLevelThreshold);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarIndicatorMode", (uint32_t&)uBossBarIndicatorMode);
		ReadFloatSetting(mcm, "BossBars", "fBossBarPhantomDuration", fBossBarPhantomDuration);
		ReadFloatSetting(mcm, "BossBars", "fBossBarDamageCounterDuration", fBossBarDamageCounterDuration);
		ReadFloatSetting(mcm, "BossBars", "fBossBarScale", fBossBarScale);
		ReadBoolSetting(mcm, "BossBars", "bBossBarUseHUDOpacity", bBossBarUseHUDOpacity);
		ReadFloatSetting(mcm, "BossBars", "fBossBarOpacity", fBossBarOpacity);
		ReadFloatSetting(mcm, "BossBars", "fBossBarWidth", fBossBarWidth);
		ReadFloatSetting(mcm, "BossBars", "fBossBarResourceWidth", fBossBarResourceWidth);
		ReadFloatSetting(mcm, "BossBars", "fBossBarAnchorX", fBossBarAnchorX);
		ReadFloatSetting(mcm, "BossBars", "fBossBarAnchorY", fBossBarAnchorY);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarModifyHUD", (uint32_t&)uBossBarModifyHUD);
		ReadBoolSetting(mcm, "BossBars", "bDisplayStandaloneShoutWidgetWhenHidingCompass", bDisplayStandaloneShoutWidgetWhenHidingCompass);
		ReadUInt32Setting(mcm, "BossBars", "uBossBarMaxCount", uBossBarMaxCount);
		ReadFloatSetting(mcm, "BossBars", "fMultipleBossBarsOffset", fMultipleBossBarsOffset);
		ReadUInt32Setting(mcm, "BossBars", "uMultipleBossBarsStackDirection", (uint32_t&)uMultipleBossBarsStackDirection);

		// Player Widget
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetHealthBarDirection", (uint32_t&)uPlayerWidgetHealthBarDirection);
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetMagickaBarDirection", (uint32_t&)uPlayerWidgetMagickaBarDirection);
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetStaminaBarDirection", (uint32_t&)uPlayerWidgetStaminaBarDirection);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetDisplayPhantomBars", bPlayerWidgetDisplayPhantomBars);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetDisplaySpecialPhantomBar", bPlayerWidgetDisplaySpecialPhantomBar);
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetHealthMode", (uint32_t&)uPlayerWidgetHealthMode);
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetMagickaMode", (uint32_t&)uPlayerWidgetMagickaMode);
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetStaminaMode", (uint32_t&)uPlayerWidgetStaminaMode);
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetShoutIndicatorMode", (uint32_t&)uPlayerWidgetShoutIndicatorMode);
		ReadUInt32Setting(mcm, "PlayerWidget", "uPlayerWidgetSpecialMode", (uint32_t&)uPlayerWidgetSpecialMode);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetHideVanillaBars", bPlayerWidgetHideVanillaBars);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetForceHideVanillaBars", bPlayerWidgetForceHideVanillaBars);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetDisplayEnchantmentChargeMeter", bPlayerWidgetDisplayEnchantmentChargeMeter);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetDisplayMountStamina", bPlayerWidgetDisplayMountStamina);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetPhantomDuration", fPlayerWidgetPhantomDuration);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetScale", fPlayerWidgetScale);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetUseHUDOpacity", bPlayerWidgetUseHUDOpacity);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetOpacity", fPlayerWidgetOpacity);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetBarWidth", fPlayerWidgetBarWidth);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetSpecialBarWidth", fPlayerWidgetSpecialBarWidth);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetScaleBars", bPlayerWidgetScaleBars);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetMinBarWidth", fPlayerWidgetMinBarWidth);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetMaxBarWidth", fPlayerWidgetMaxBarWidth);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetHealthBarScaleMult", fPlayerWidgetHealthBarScaleMult);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetMagickaBarScaleMult", fPlayerWidgetMagickaBarScaleMult);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetStaminaBarScaleMult", fPlayerWidgetStaminaBarScaleMult);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetX", fPlayerWidgetX);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetY", fPlayerWidgetY);
		ReadBoolSetting(mcm, "PlayerWidget", "bPlayerWidgetCombined", bPlayerWidgetCombined);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetHealthX", fPlayerWidgetHealthX);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetHealthY", fPlayerWidgetHealthY);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetMagickaX", fPlayerWidgetMagickaX);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetMagickaY", fPlayerWidgetMagickaY);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetStaminaX", fPlayerWidgetStaminaX);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetStaminaY", fPlayerWidgetStaminaY);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetEnchantmentChargeMeterX", fPlayerWidgetEnchantmentChargeMeterX);
		ReadFloatSetting(mcm, "PlayerWidget", "fPlayerWidgetEnchantmentChargeMeterY", fPlayerWidgetEnchantmentChargeMeterY);

		// Recent Loot
		ReadBoolSetting(mcm, "RecentLoot", "bRecentLootHideVanillaMessage", bRecentLootHideVanillaMessage);
		ReadBoolSetting(mcm, "RecentLoot", "bRecentLootHideInInventoryMenus", bRecentLootHideInInventoryMenus);
		ReadBoolSetting(mcm, "RecentLoot", "bRecentLootHideInCraftingMenus", bRecentLootHideInCraftingMenus);
		ReadUInt32Setting(mcm, "RecentLoot", "uRecentLootMaxMessageCount", uRecentLootMaxMessageCount);
		ReadFloatSetting(mcm, "RecentLoot", "fRecentLootMessageDuration", fRecentLootMessageDuration);
		ReadFloatSetting(mcm, "RecentLoot", "fRecentLootScale", fRecentLootScale);
		ReadBoolSetting(mcm, "RecentLoot", "bRecentLootUseHUDOpacity", bRecentLootUseHUDOpacity);
		ReadFloatSetting(mcm, "RecentLoot", "fRecentLootOpacity", fRecentLootOpacity);
		ReadUInt32Setting(mcm, "RecentLoot", "uRecentLootListDirection", (uint32_t&)uRecentLootListDirection);
		ReadFloatSetting(mcm, "RecentLoot", "fRecentLootX", fRecentLootX);
		ReadFloatSetting(mcm, "RecentLoot", "fRecentLootY", fRecentLootY);

		// Floating text
		ReadBoolSetting(mcm, "FloatingText", "bEnableFloatingCombatText", bEnableFloatingCombatText);
		ReadBoolSetting(mcm, "FloatingText", "bFloatingTextScaleWithDistance", bFloatingTextScaleWithDistance);
		ReadFloatSetting(mcm, "FloatingText", "fFloatingTextScale", fFloatingTextScale);

		// Colors
		ReadColorStringSetting(mcm, "Colors", "sHealthColor", uHealthColor);
		ReadColorStringSetting(mcm, "Colors", "sHealthPhantomColor", uHealthPhantomColor);
		ReadColorStringSetting(mcm, "Colors", "sHealthBackgroundColor", uHealthBackgroundColor);
		ReadColorStringSetting(mcm, "Colors", "sHealthPenaltyColor", uHealthPenaltyColor);
		ReadColorStringSetting(mcm, "Colors", "sHealthFlashColor", uHealthFlashColor);

		ReadColorStringSetting(mcm, "Colors", "sMagickaColor", uMagickaColor);
		ReadColorStringSetting(mcm, "Colors", "sMagickaPhantomColor", uMagickaPhantomColor);
		ReadColorStringSetting(mcm, "Colors", "sMagickaBackgroundColor", uMagickaBackgroundColor);
		ReadColorStringSetting(mcm, "Colors", "sMagickaPenaltyColor", uMagickaPenaltyColor);
		ReadColorStringSetting(mcm, "Colors", "sMagickaFlashColor", uMagickaFlashColor);

		ReadColorStringSetting(mcm, "Colors", "sStaminaColor", uStaminaColor);
		ReadColorStringSetting(mcm, "Colors", "sStaminaPhantomColor", uStaminaPhantomColor);
		ReadColorStringSetting(mcm, "Colors", "sStaminaBackgroundColor", uStaminaBackgroundColor);
		ReadColorStringSetting(mcm, "Colors", "sStaminaPenaltyColor", uStaminaPenaltyColor);
		ReadColorStringSetting(mcm, "Colors", "sStaminaFlashColor", uStaminaFlashColor);

		ReadColorStringSetting(mcm, "Colors", "sSpecialColor", uSpecialColor);
		ReadColorStringSetting(mcm, "Colors", "sSpecialPhantomColor", uSpecialPhantomColor);
		ReadColorStringSetting(mcm, "Colors", "sSpecialBackgroundColor", uSpecialBackgroundColor);
		ReadColorStringSetting(mcm, "Colors", "sSpecialPenaltyColor", uSpecialPenaltyColor);
		ReadColorStringSetting(mcm, "Colors", "sSpecialFlashColor", uSpecialFlashColor);

		ReadColorStringSetting(mcm, "Colors", "sDefaultColor", uDefaultColor);
		ReadColorStringSetting(mcm, "Colors", "sDefaultColorOutline", uDefaultColorOutline);
		ReadColorStringSetting(mcm, "Colors", "sWeakerColor", uWeakerColor);
		ReadColorStringSetting(mcm, "Colors", "sWeakerColorOutline", uWeakerColorOutline);
		ReadColorStringSetting(mcm, "Colors", "sStrongerColor", uStrongerColor);
		ReadColorStringSetting(mcm, "Colors", "sStrongerColorOutline", uStrongerColorOutline);
		ReadColorStringSetting(mcm, "Colors", "sTeammateColor", uTeammateColor);
		ReadColorStringSetting(mcm, "Colors", "sTeammateColorOutline", uTeammateColorOutline);
	};

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Reading MCM .ini...");

	readMCM(defaultSettingsPath); // read the default ini first
	readMCM(mcmPath);

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");

	HUDHandler::GetSingleton()->OnSettingsUpdated();
}

void Settings::OnPostLoadGame()
{
	if(glob_trueHUDVersion) {
		glob_trueHUDVersion->value = 1.f;
	}

	if (glob_trueHUDSpecialResourceBars) {
		glob_trueHUDSpecialResourceBars->value = Messaging::TrueHUDInterface::GetSingleton()->IsSpecialResourceBarsControlTaken();
	}
}

void Settings::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound)
	{
		a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
	}
}

void Settings::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		a_setting = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
	}
}

void Settings::ReadUInt32Setting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		a_setting = static_cast<uint32_t>(a_ini.GetLongValue(a_sectionName, a_settingName));
	}
}

void Settings::ReadColorStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
	const char* value = nullptr;

	constexpr const char prefix1[] = "0x";
	constexpr const char prefix2[] = "#";
	constexpr const char cset[] = "0123456789ABCDEFabcdef";

	value = a_ini.GetValue(a_sectionName, a_settingName);
	if (value) 
	{
		std::string str = value;

		if (!strncasecmp(str.c_str(), prefix1, sizeof(prefix1)))
		{
			str = str.substr(sizeof(prefix1), str.size());
		}

		if (!strncasecmp(str.c_str(), prefix2, sizeof(prefix2)))
		{
			str = str.substr(sizeof(prefix2), str.size());
		}
		
		bool bMatches = std::strspn(str.data(), cset) == str.size();

		if (bMatches) 
		{
			a_setting = std::stoi(str.data(), 0, 16);
		} 
		else 
		{
			const auto skyrimVM = ConsoleRE::SkyrimVM::GetSingleton();
			auto vm = skyrimVM ? skyrimVM->impl : nullptr;
			if (vm) 
			{
				ConsoleRE::BSFixedString modName{ "TrueHUD" };
				std::string settingStr = a_settingName;
				settingStr.append(":");
				settingStr.append(a_sectionName);
				ConsoleRE::BSFixedString setting = settingStr.c_str();
				ConsoleRE::BSFixedString settingValue = "0xFFFFFF";
				ConsoleRE::BSTSmartPointer<ConsoleRE::BSScript::IStackCallbackFunctor> callback;
				auto vmargs = ConsoleRE::MakeFunctionArguments(std::move(modName), std::move(setting), std::move(settingValue));
				vm->DispatchStaticCall("MCM", "SetModSettingString", vmargs, callback);
				delete vmargs;
			}
		}
	}
}
