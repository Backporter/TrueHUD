#include "Widgets/ActorInfoBar.h"
#include "Offsets.h"
#include "Settings.h"
#include "Utils.h"
#include "HUDHandler.h"
#include "Scaleform/Scaleform.h"

namespace Scaleform
{
	void ActorInfoBar::Update([[maybe_unused]] float a_deltaTime)
	{
		ConsoleRE::GFxValue result;
		_object.Invoke("isReadyToRemove", &result);
		if (result.GetBool()) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		UpdatePosition();
		UpdateInfo();
	}

	void ActorInfoBar::UpdatePosition()
	{
		if (!_refHandle) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		// don't update position if alpha is zero
		/*ConsoleRE::GFxValue::DisplayInfo dInfo;
		object.GetDisplayInfo(&dInfo);
		if (dInfo.GetAlpha() == 0) {
			return;
		}*/

		/*ConsoleRE::GFxValue alpha;
		object.GetMember("_alpha", &alpha);
		if (alpha.GetNumber() == 0) {
			return;
		}*/

		ConsoleRE::GRectF rect = _view->GetVisibleFrameRect();

		ConsoleRE::NiPoint3 targetPos;
		Utils::GetTargetPos(_refHandle, targetPos, Settings::uInfoBarAnchor == WidgetAnchor::kBody);
		targetPos.z += Settings::fInfoBarOffsetZ;
		ConsoleRE::NiPoint2 screenPos;

		ConsoleRE::NiCamera::WorldPtToScreenPt3((float(*)[4])g_worldToCamMatrix, *g_viewPort, targetPos, screenPos.x, screenPos.y, _depth, 1e-5f);

		_bIsOffscreen = _depth < 0;

		if (_bIsOffscreen) {
			ConsoleRE::GFxValue alpha;
			_object.GetMember("_alpha", &alpha);
			if (alpha.GetNumber() == 0) {
				return;
			}
		}

		float fNear = *g_fNear;
		float fFar = *g_fFar;
		float linearizedDepth = fNear * fFar / (fFar + _depth * (fNear - fFar));

		screenPos.y = 1.0f - screenPos.y;  // Flip y for Flash coordinate system
		screenPos.y = rect.top + (rect.bottom - rect.top) * screenPos.y;
		screenPos.x = rect.left + (rect.right - rect.left) * screenPos.x;

		float scale;

		if (Settings::bInfoBarScaleWithDistance) {
			float scaleMult = fmin(fmax(linearizedDepth, 500.f), 2000.f);
			float remapped = (((scaleMult - 500.f) * (50.f - 100.f)) / (2000.f - 500.f)) + 100.f;
			scale = remapped;
		} else {
			scale = 100.f;
		}

		scale *= Settings::fInfoBarScale;

		ConsoleRE::GFxValue::DisplayInfo displayInfo;
		displayInfo.SetPosition(screenPos.x, screenPos.y);
		displayInfo.SetScale(scale, scale);
		_object.SetDisplayInfo(displayInfo);
	}

	void ActorInfoBar::LoadConfig()
	{
		using ColorType = TRUEHUD_API::BarColorType;

		auto hudHandler = HUDHandler::GetSingleton();
		auto hudMenu = HUDHandler::GetTrueHUDMenu();

		ConsoleRE::GFxValue args[44];
		args[0].SetNumber(static_cast<uint32_t>(Settings::uInfoBarDirection));
		args[1].SetNumber(static_cast<uint32_t>(Settings::uInfoBarDisplayName));
		args[2].SetBoolean(Settings::bInfoBarDisplayPhantomBars);
		args[3].SetBoolean(Settings::bInfoBarDisplaySpecialPhantomBar);
		args[4].SetNumber(static_cast<uint32_t>(Settings::uInfoBarDisplayDamageCounter));
		args[5].SetNumber(static_cast<uint32_t>(Settings::uInfoBarDisplayIndicator));
		args[6].SetNumber(static_cast<uint32_t>(Settings::uInfoBarDamageCounterAlignment));
		args[7].SetNumber(static_cast<uint32_t>(Settings::uInfoBarResourcesModeTarget));
		args[8].SetNumber(static_cast<uint32_t>(Settings::uInfoBarResourcesModeHostiles));
		args[9].SetNumber(static_cast<uint32_t>(Settings::uInfoBarResourcesModeTeammates));
		args[10].SetNumber(static_cast<uint32_t>(Settings::uInfoBarResourcesModeOthers));
		args[11].SetNumber(static_cast<uint32_t>(Settings::uInfoBarSpecialModeTarget));
		args[12].SetNumber(static_cast<uint32_t>(Settings::uInfoBarSpecialModeHostiles));
		args[13].SetNumber(static_cast<uint32_t>(Settings::uInfoBarSpecialModeTeammates));
		args[14].SetNumber(static_cast<uint32_t>(Settings::uInfoBarSpecialModeOthers));
		args[15].SetBoolean(hudHandler->bSpecialMode);
		args[16].SetNumber(static_cast<uint32_t>(Settings::uInfoBarIndicatorMode));	
		args[17].SetNumber(Settings::fInfoBarPhantomDuration);
		args[18].SetNumber(Settings::fInfoBarDamageCounterDuration);
		args[19].SetNumber((Settings::bInfoBarUseHUDOpacity ? *g_fHUDOpacity : Settings::fInfoBarOpacity) * 100.f);
		args[20].SetNumber(Settings::fInfoBarWidth);
		args[21].SetNumber(Settings::fInfoBarResourceWidth);
		args[22].SetNumber(Settings::fInfoBarTargetWidth);
		args[23].SetNumber(Settings::fInfoBarTargetResourceWidth);
		args[24].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::BarColor));
		args[25].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::PhantomColor));
		args[26].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::BackgroundColor));
		args[27].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::PenaltyColor));
		args[28].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kHealth, ColorType::FlashColor));
		args[29].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::BarColor));
		args[30].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::PhantomColor));
		args[31].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::BackgroundColor));
		args[32].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::PenaltyColor));
		args[33].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kMagicka, ColorType::FlashColor));
		args[34].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::BarColor));
		args[35].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::PhantomColor));
		args[36].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::BackgroundColor));
		args[37].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::PenaltyColor));
		args[38].SetNumber(hudMenu->GetBarColor(_refHandle, ConsoleRE::ActorValue::kStamina, ColorType::FlashColor));
		args[39].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::BarColor));
		args[40].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::PhantomColor));
		args[41].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::BackgroundColor));
		args[42].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::PenaltyColor));
		args[43].SetNumber(hudMenu->GetSpecialBarColor(_refHandle, ColorType::FlashColor));
		_object.Invoke("loadConfig", nullptr, args, 44);
	}
}
