#include "Widgets/ShoutIndicator.h"
#include "Offsets.h"
#include "Settings.h"
#include "Utils.h"

namespace Scaleform
{
	void ShoutIndicator::Update([[maybe_unused]] float a_deltaTime)
	{
		UpdatePosition();
		
		SetPercent(GetPercent());
	}

	void ShoutIndicator::Initialize()
	{
		LoadConfig();
	}

	void ShoutIndicator::Dispose()
	{
		_object.Invoke("cleanUp", nullptr, nullptr, 0);
	}

	void ShoutIndicator::SetPercent(float a_pct)
	{
		ConsoleRE::GFxValue arg;
		arg.SetNumber(a_pct * 0.01f);

		_object.Invoke("setProgress", nullptr, &arg, 1);
	}

	void ShoutIndicator::FlashShoutWidget()
	{
		ConsoleRE::GFxValue arg;
		arg.SetBoolean(true);

		_object.Invoke("playFlash", nullptr, &arg, 1);
	}

	void ShoutIndicator::UpdatePosition()
	{
		ConsoleRE::GRectF rect = _view->GetVisibleFrameRect();

		ConsoleRE::NiPoint2 screenPos;

		auto def = _view->GetMovieDef();
		if (def) {
			screenPos.x = def->GetWidth();
			screenPos.y = def->GetHeight();
		}

		screenPos.x *= Settings::fBossBarAnchorX;
		screenPos.x += (Settings::fBossBarWidth * 0.5f + 50.f);
		screenPos.y *= Settings::fBossBarAnchorY;

		float scale = 50.f;

		ConsoleRE::GFxValue::DisplayInfo displayInfo;
		displayInfo.SetPosition(screenPos.x, screenPos.y);
		displayInfo.SetScale(scale, scale);
		_object.SetDisplayInfo(displayInfo);
	}

	void ShoutIndicator::LoadConfig()
	{
		ConsoleRE::GFxValue arg;
		arg.SetNumber((Settings::bBossBarUseHUDOpacity ? *g_fHUDOpacity : Settings::fBossBarOpacity) * 100.f);
		_object.Invoke("loadConfig", nullptr, &arg, 1);
	}

	float ShoutIndicator::GetPercent()
	{
		auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
		auto currentProcess = playerCharacter->currentProcess;
		
		float voiceRecoveryTime = 0.f;
		float cooldown = _cooldown;

		if (currentProcess && currentProcess->high) {
			voiceRecoveryTime = currentProcess->high->voiceRecoveryTime;
		}

		if (voiceRecoveryTime > 1e-4f) {
			if (cooldown <= voiceRecoveryTime) {
				cooldown = voiceRecoveryTime;
			}
		} else {
			cooldown = 0.f;
		}

		_cooldown = cooldown;

		if (cooldown <= 0.f) {
			return 100.f;
		}

		float endDuration = *g_fShoutMeterEndDuration;

		if (voiceRecoveryTime <= endDuration) {
			return 80.f - ((voiceRecoveryTime - endDuration) / endDuration) * 20.f;
		}

		return (voiceRecoveryTime - cooldown) / (endDuration - cooldown) * 80.f;
	}
}
