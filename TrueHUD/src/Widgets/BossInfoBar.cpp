#include "Widgets/BossInfoBar.h"
#include "Offsets.h"
#include "Settings.h"
#include "HUDHandler.h"

namespace Scaleform
{
	void BossInfoBar::Update([[maybe_unused]] float a_deltaTime)
	{
		ConsoleRE::GFxValue result;
		_object.Invoke("isReadyToRemove", &result);
		if (result.GetBool()) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		UpdateInfo();
	}

	void BossInfoBar::Initialize()
	{
		InfoBarBase::Initialize();
		UpdatePosition();
	}

	int32_t BossInfoBar::GetIndex() const
	{
		return _index;
	}

	void BossInfoBar::SetIndex(int32_t a_index)
	{
		_index = a_index;
		UpdatePosition();
	}

	void BossInfoBar::UpdatePosition()
	{
		if (!_refHandle) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		ConsoleRE::GRectF rect = _view->GetVisibleFrameRect();

		//bIsOffscreen = depth < 0;

		ConsoleRE::NiPoint2 screenPos;
		float offsetY = _index * Settings::fMultipleBossBarsOffset;

		auto def = _view->GetMovieDef();
		if (def) {
			screenPos.x = def->GetWidth();
			screenPos.y = def->GetHeight();
			offsetY /= def->GetHeight();
		}

		if (Settings::uMultipleBossBarsStackDirection == VerticalDirection::kUp) {
			offsetY *= -1;
		}

		screenPos.x *= Settings::fBossBarAnchorX;
		screenPos.y *= Settings::fBossBarAnchorY + offsetY;

		float scale = Settings::fBossBarScale * 100.f;

		ConsoleRE::GFxValue::DisplayInfo displayInfo;
		displayInfo.SetPosition(screenPos.x, screenPos.y);
		displayInfo.SetScale(scale, scale);
		_object.SetDisplayInfo(displayInfo);
	}

	void BossInfoBar::LoadConfig()
	{
		using ColorType = TRUEHUD_API::BarColorType;

		auto hudHandler = HUDHandler::GetSingleton();
		auto hudMenu = HUDHandler::GetTrueHUDMenu();

		ConsoleRE::GFxValue args[37];
		args[0].SetNumber(static_cast<uint32_t>(Settings::uBossBarDirection));
		args[1].SetBoolean(Settings::bBossBarDisplayPhantomBars);
		args[2].SetBoolean(Settings::bBossBarDisplaySpecialPhantomBar);
		args[3].SetBoolean(Settings::bBossBarDisplayDamageCounter);
		args[4].SetBoolean(Settings::bBossBarDisplayIndicator);
		args[5].SetNumber(static_cast<uint32_t>(Settings::uBossBarDamageCounterAlignment));
		args[6].SetNumber(static_cast<uint32_t>(Settings::uBossBarResourcesMode));
		args[7].SetNumber(static_cast<uint32_t>(Settings::uBossBarSpecialMode));
		args[8].SetBoolean(hudHandler->bSpecialMode);
		args[9].SetNumber(static_cast<uint32_t>(Settings::uBossBarResourceAlignment));
		args[10].SetNumber(static_cast<uint32_t>(Settings::uBossBarNameAlignment));
		args[11].SetNumber(static_cast<uint32_t>(Settings::uBossBarIndicatorMode));
		args[12].SetNumber(Settings::fBossBarPhantomDuration);
		args[13].SetNumber(Settings::fBossBarDamageCounterDuration);
		args[14].SetNumber((Settings::bBossBarUseHUDOpacity ? *g_fHUDOpacity : Settings::fBossBarOpacity) * 100.f);
		args[15].SetNumber(Settings::fBossBarWidth);
		args[16].SetNumber(Settings::fBossBarResourceWidth);
		args[17].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::BarColor));
		args[18].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::PhantomColor));
		args[19].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::BackgroundColor));
		args[20].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::PenaltyColor));
		args[21].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::FlashColor));
		args[22].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::BarColor));
		args[23].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::PhantomColor));
		args[24].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::BackgroundColor));
		args[25].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::PenaltyColor));
		args[26].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::FlashColor));
		args[27].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::BarColor));
		args[28].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::PhantomColor));
		args[29].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::BackgroundColor));
		args[30].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::PenaltyColor));
		args[31].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::FlashColor));
		args[32].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::BarColor));
		args[33].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::PhantomColor));
		args[34].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::BackgroundColor));
		args[35].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::PenaltyColor));
		args[36].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::FlashColor));
		_object.Invoke("loadConfig", nullptr, args, 37);
	}
}
