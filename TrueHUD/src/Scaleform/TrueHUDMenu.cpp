#include "Scaleform/TrueHUDMenu.h"

#include "Settings.h"
#include "Offsets.h"
#include "HUDHandler.h"
#include "Widgets/ActorInfoBar.h"
#include "Widgets/FloatingText.h"
#include "Utils.h"

#define max(a, b) (a < b) ? b : a;

namespace Scaleform
{
	void TrueHUDMenu::Register()
	{
		auto ui = ConsoleRE::UI::GetSingleton();
		if (ui) 
		{
			ui->Register(MENU_NAME, Creator);
			xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", MENU_NAME);
		}
	}

	bool TrueHUDMenu::IsOpen() const
	{
		return _bIsOpen;
	}

	ConsoleRE::GPtr<ConsoleRE::GFxMovieView> TrueHUDMenu::GetView() const
	{
		return _view;
	}

	void TrueHUDMenu::SetTarget(ConsoleRE::ObjectRefHandle a_actorHandle)
	{
		_targetHandle = a_actorHandle;

		if (Settings::bEnableActorInfoBars) {
			if (a_actorHandle) {
				AddActorInfoBar(a_actorHandle);
			}
		}		
	}

	void TrueHUDMenu::SetSoftTarget(ConsoleRE::ObjectRefHandle a_actorHandle)
	{
		_softTargetHandle = a_actorHandle;

		if (Settings::bEnableActorInfoBars) {
			if (a_actorHandle) {
				AddActorInfoBar(a_actorHandle);
			}
		}		
	}

	ConsoleRE::ObjectRefHandle TrueHUDMenu::GetTarget() const
	{
		return _targetHandle;
	}

	ConsoleRE::ObjectRefHandle TrueHUDMenu::GetSoftTarget() const
	{
		return _softTargetHandle;
	}

	bool TrueHUDMenu::HasActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
	{
		Locker locker(_lock);

		return _actorInfoBarMap.find(a_actorHandle) != _actorInfoBarMap.end();
	}

	bool TrueHUDMenu::AddActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
	{	
		using WidgetStateMode = InfoBarBase::WidgetStateMode;

		if (_view && !HasActorInfoBar(a_actorHandle) && !HasBossInfoBar(a_actorHandle)) {
			Locker locker(_lock);
			auto widget = std::make_shared<ActorInfoBar>(_view, a_actorHandle);

			auto iter = _actorInfoBarMap.emplace(a_actorHandle, widget);
			if (iter.second) {
				ConsoleRE::GFxValue arg;
				arg.SetNumber(widget->_widgetID);

				ConsoleRE::GFxValue obj;
				_view->Invoke("_root.TrueHUD.AddInfoBarWidget", &obj, &arg, 1);
				if (!obj.IsDisplayObject()) {
					_actorInfoBarMap.erase(iter.first);
					return false;
				}
				widget->_object = obj;
				widget->Initialize();
				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::RemoveActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode)
	{
		using WidgetStateMode = InfoBarBase::WidgetStateMode;

		if (_view) {
			Locker locker(_lock);
			auto it = _actorInfoBarMap.find(a_actorHandle);
			if (it != _actorInfoBarMap.end()) {
				auto& widget = it->second;

				switch (a_removalMode)
				{
				case WidgetRemovalMode::Immediate:
					if (widget->_object.IsDisplayObject()) {
						ConsoleRE::GFxValue arg;
						arg.SetNumber(widget->_widgetID);

						widget->Dispose();
						_view->Invoke("_root.TrueHUD.RemoveInfoBarWidget", nullptr, &arg, 1);
					}
					_actorInfoBarMap.erase(it);
					break;

				case WidgetRemovalMode::Normal:
					widget->SetWidgetState(WidgetStateMode::kRemove);
					break;

				case WidgetRemovalMode::Delayed:
					widget->SetWidgetState(WidgetStateMode::kTargetKilled);
					break;
				}

				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::HasBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
	{
		Locker locker(_lock);

		bool bInQueue = std::find(_bossQueue.begin(), _bossQueue.end(), a_actorHandle) != _bossQueue.end();

		return bInQueue || _bossInfoBarMap.find(a_actorHandle) != _bossInfoBarMap.end();
	}

	bool TrueHUDMenu::AddBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
	{
		using WidgetStateMode = InfoBarBase::WidgetStateMode;

		if (_view && !HasBossInfoBar(a_actorHandle)) {
			if (_bossInfoBarMap.size() < Settings::uBossBarMaxCount) {
				// Add a boss bar
				return AddBossInfoBarWidget(a_actorHandle);
			} else {
				// Maximum count of boss bars, add to queue
				_bossQueue.emplace_back(a_actorHandle);
			}			
		}

		return false;
	}

	bool TrueHUDMenu::RemoveBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode)
	{
		using WidgetStateMode = InfoBarBase::WidgetStateMode;

		if (_view) {
			Locker locker(_lock);

			// Remove from queue
			_bossQueue.remove(a_actorHandle);

			auto it = _bossInfoBarMap.find(a_actorHandle);
			if (it != _bossInfoBarMap.end()) {
				// Remove widget
				auto& widget = it->second;

				switch (a_removalMode) {
				case WidgetRemovalMode::Immediate:
					{
						auto index = widget->GetIndex();
						if (widget->_object.IsDisplayObject()) {
							ConsoleRE::GFxValue arg;
							arg.SetNumber(widget->_widgetID);

							widget->Dispose();
							_view->Invoke("_root.TrueHUD.RemoveBossInfoBarWidget", nullptr, &arg, 1);
						}
						_bossInfoBarMap.erase(it);
						RefreshBossBarIndexes(index);
						UpdateBossQueue();
						break;
					}

				case WidgetRemovalMode::Normal:
					{
						widget->SetWidgetState(WidgetStateMode::kRemove);
						break;
					}

				case WidgetRemovalMode::Delayed:
					{
						widget->SetWidgetState(WidgetStateMode::kTargetKilled);
						break;
					}
				}

				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::AddShoutIndicator()
	{
		if (_view) {
			Locker locker(_lock);

			if (_shoutIndicator) {
				RemoveShoutIndicator();
			}

			_shoutIndicator = std::make_shared<ShoutIndicator>(_view);

			if (_shoutIndicator) {
				ConsoleRE::GFxValue obj;
				_view->Invoke("_root.TrueHUD.AddShoutIndicatorWidget", &obj, nullptr, 0);
				if (!obj.IsDisplayObject()) {
					_shoutIndicator = nullptr;
					return false;
				} else {
					_shoutIndicator->_object = obj;
					_shoutIndicator->Initialize();
				}

				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::RemoveShoutIndicator()
	{
		if (_view) {
			Locker locker(_lock);
			if (_shoutIndicator) {
				if (_shoutIndicator->_object.IsDisplayObject()) {
					_shoutIndicator->Dispose();
					_view->Invoke("_root.TrueHUD.RemoveShoutIndicatorWidget", nullptr, nullptr, 0);
				}
				_shoutIndicator = nullptr;

				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::AddPlayerWidget()
	{
		if (_view) {
			Locker locker(_lock);

			if (_playerWidget) {
				RemovePlayerWidget();
			}

			_playerWidget = std::make_shared<PlayerWidget>(_view);

			if (_playerWidget) {
				ConsoleRE::GFxValue obj;
				_view->Invoke("_root.TrueHUD.AddPlayerWidget", &obj, nullptr, 0);
				if (!obj.IsDisplayObject()) {
					_playerWidget = nullptr;
					return false;
				}

				_playerWidget->_object = obj;
				_playerWidget->Initialize();
				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::RemovePlayerWidget()
	{
		if (_view) {
			Locker locker(_lock);
			if (_playerWidget) {
				if (_playerWidget->_object.IsDisplayObject()) {
					_playerWidget->Dispose();
					_view->Invoke("_root.TrueHUD.RemovePlayerWidget", nullptr, nullptr, 0);
				}
				_playerWidget = nullptr;

				return true;
			}
		}

		return false;
	}

	void TrueHUDMenu::UpdatePlayerWidgetChargeMeters(float a_percent, bool a_bForce, bool a_bLeftHand, bool a_bShow)
	{
		if (_playerWidget) {
			_playerWidget->UpdatePlayerWidgetChargeMeters(a_percent, a_bForce, a_bLeftHand, a_bShow);
		}
	}

	bool TrueHUDMenu::AddRecentLootWidget()
	{
		if (_view) {
			Locker locker(_lock);

			if (_recentLoot) {
				RemoveRecentLootWidget();
			}

			_recentLoot = std::make_shared<RecentLoot>(_view);

			if (_recentLoot) {
				ConsoleRE::GFxValue obj;
				_view->Invoke("_root.TrueHUD.AddRecentLootWidget", &obj, nullptr, 0);
				if (!obj.IsDisplayObject()) {
					_recentLoot = nullptr;
					return false;
				}

				_recentLoot->_object = obj;
				_recentLoot->Initialize();
				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::RemoveRecentLootWidget()
	{
		if (_view) {
			Locker locker(_lock);
			if (_recentLoot) {
				if (_recentLoot->_object.IsDisplayObject()) {
					_recentLoot->Dispose();
					_view->Invoke("_root.TrueHUD.RemoveRecentLootWidget", nullptr, nullptr, 0);
				}
				_recentLoot = nullptr;

				return true;
			}
		}

		return false;
	}

	void TrueHUDMenu::AddRecentLootMessage(ConsoleRE::TESBoundObject* a_object, const char* a_name, uint32_t a_count)
	{
		if (_recentLoot) 
		{
			_recentLoot->AddMessage(a_object, a_name, a_count);
		}
	}

	bool TrueHUDMenu::AddFloatingWorldTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint3 a_worldPosition)
	{
		if (_view) 
		{
			Locker locker(_lock);
			auto widget = std::make_shared<FloatingText>(_view, _nextFloatingTextID, a_text, a_color, a_duration, a_bSpecial, a_worldPosition);
			auto iter = _floatingTextMap.emplace(_nextFloatingTextID, widget);
			++_nextFloatingTextID;
			if (iter.second) 
			{
				ConsoleRE::GFxValue arg;
				arg.SetNumber(widget->_widgetID);

				ConsoleRE::GFxValue obj;
				_view->Invoke("_root.TrueHUD.AddFloatingTextWidget", &obj, &arg, 1);
				if (!obj.IsDisplayObject()) {
					_floatingTextMap.erase(iter.first);
					return false;
				}

				widget->_object = obj;
				widget->Initialize();
				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::AddFloatingScreenTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint2 a_screenPosition)
	{
		if (_view) {
			Locker locker(_lock);
			auto widget = std::make_shared<FloatingText>(_view, _nextFloatingTextID, a_text, a_color, a_duration, a_bSpecial, a_screenPosition);
			auto iter = _floatingTextMap.emplace(_nextFloatingTextID, widget);
			++_nextFloatingTextID;
			if (iter.second) {
				ConsoleRE::GFxValue arg;
				arg.SetNumber(widget->_widgetID);

				ConsoleRE::GFxValue obj;
				_view->Invoke("_root.TrueHUD.AddFloatingTextWidget", &obj, &arg, 1);
				if (!obj.IsDisplayObject()) {
					_floatingTextMap.erase(iter.first);
					return false;
				}

				widget->_object = obj;
				widget->Initialize();
				return true;
			}
		}

		return false;
	}

	bool TrueHUDMenu::LoadCustomWidgets(size_t a_myPluginHandle, const char* a_filePath, APIResultCallback&& a_successCallback)
	{
		if (_view) {
			ConsoleRE::GFxValue args[2];
			args[0].SetNumber(a_myPluginHandle);
			args[1].SetString(a_filePath);
			ConsoleRE::GFxValue result;

			_view->Invoke("_root.TrueHUD.LoadCustomWidgets", &result, args, 2);

			bool bSuccess = result.GetBool();

			if (a_successCallback) {
				a_successCallback(bSuccess ? TRUEHUD_API::APIResult::OK : TRUEHUD_API::APIResult::WidgetFailedToLoad);
			}

			return bSuccess;
		}

		return false;
	}

	bool TrueHUDMenu::RegisterNewWidgetType(size_t a_myPluginHandle, uint32_t a_widgetType)
	{
		if (_view) {
			ConsoleRE::GFxValue args[2];
			args[0].SetNumber(a_myPluginHandle);
			args[1].SetNumber(a_widgetType);
			
			_view->Invoke("_root.TrueHUD.RegisterNewWidgetType", nullptr, args, 2);
		}

		return false;
	}

	bool TrueHUDMenu::AddWidget(size_t a_myPluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, const char* a_symbolIdentifier, std::shared_ptr<WidgetBase> a_widget)
	{
		if (_view && a_widget) {
			Locker locker(_lock);
			ConsoleRE::GFxValue args[4];
			args[0].SetNumber(a_myPluginHandle);
			args[1].SetNumber(a_widgetType);
			args[2].SetNumber(a_widgetID);
			args[3].SetString(a_symbolIdentifier);

			ConsoleRE::GFxValue obj;
			_view->Invoke("_root.TrueHUD.AddCustomWidget", &obj, args, 4);
			if (!obj.IsDisplayObject()) {
				return false;
			}
			a_widget->_object = obj;
			_customWidgets[a_myPluginHandle][a_widgetType].emplace(a_widgetID, a_widget);
			a_widget->_view = _view;

			a_widget->Initialize();
			return true;
		}

		return false;
	}

	bool TrueHUDMenu::RemoveWidget(size_t a_myPluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, WidgetRemovalMode a_removalMode)
	{
		using WidgetState = TRUEHUD_API::WidgetBase::WidgetState;

		if (_view) {
			Locker locker(_lock);

			auto it = _customWidgets.find(a_myPluginHandle);
			if (it != _customWidgets.end()) {
				auto& pluginWidgets = it->second;
				auto iter = pluginWidgets.find(a_widgetType);
				if (iter != pluginWidgets.end()) {
					auto& typeWidgets = iter->second;
					auto widgetIt = typeWidgets.find(a_widgetID);
					if (widgetIt != typeWidgets.end()) {
						auto& widget = widgetIt->second;

						switch (a_removalMode) {
						case WidgetRemovalMode::Immediate:
							if (widget->_object.IsDisplayObject()) {
								ConsoleRE::GFxValue args[3];
								args[0].SetNumber(a_myPluginHandle);
								args[1].SetNumber(a_widgetType);
								args[2].SetNumber(a_widgetID);

								widget->Dispose();
								_view->Invoke("_root.TrueHUD.RemoveCustomWidget", nullptr, args, 3);
							}
							typeWidgets.erase(widgetIt);
							break;

						case WidgetRemovalMode::Normal:
							widget->SetWidgetState(WidgetState::kPendingRemoval);
							break;

						case WidgetRemovalMode::Delayed:
							widget->SetWidgetState(WidgetState::kDelayedRemoval);
							break;
						}

						return true;
					}
				}				
			}
		}

		return false;
	}

	void TrueHUDMenu::FlashActorValue(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, bool a_bLong)
	{
		if (a_actorHandle.native_handle() == 0x100000) {  // 0x100000 == player
			if (_shoutIndicator && a_actorValue == ConsoleRE::ActorValue::kVoiceRate) {
				_shoutIndicator->FlashShoutWidget();
				return;
			}
			if (_playerWidget) {
				_playerWidget->FlashActorValue(a_actorValue, a_bLong);
				return;
			}
		}

		Locker locker(_lock);

		// try the boss bars first
		auto it = _bossInfoBarMap.find(a_actorHandle);
		if (it != _bossInfoBarMap.end()) {
			auto& widget = it->second;
			widget->FlashActorValue(a_actorValue, a_bLong);
			return;
		}

		// try normal info bars
		auto iter = _actorInfoBarMap.find(a_actorHandle);
		if (iter != _actorInfoBarMap.end()) {
			auto& widget = iter->second;
			widget->FlashActorValue(a_actorValue, a_bLong);
			return;
		}
	}

	void TrueHUDMenu::FlashActorSpecialBar(ConsoleRE::ObjectRefHandle a_actorHandle, bool a_bLong)
	{
		if (a_actorHandle.native_handle() == 0x100000 && _playerWidget) {  // 0x100000 == player
			_playerWidget->FlashSpecial(a_bLong);
			return;
		}

		Locker locker(_lock);

		// try the boss bars first
		auto it = _bossInfoBarMap.find(a_actorHandle);
		if (it != _bossInfoBarMap.end()) {
			auto& widget = it->second;
			widget->FlashSpecial(a_bLong);
			return;
		}

		// try normal info bars
		auto iter = _actorInfoBarMap.find(a_actorHandle);
		if (iter != _actorInfoBarMap.end()) {
			auto& widget = iter->second;
			widget->FlashSpecial(a_bLong);
			return;
		}
	}

	void TrueHUDMenu::OverrideBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType, uint32_t a_color)
	{
		switch (a_actorValue) {
		case ConsoleRE::ActorValue::kHealth:
			_colorOverrides[a_actorHandle][BarType::kHealth].SetOverride(a_colorType, a_color);
			break;
		case ConsoleRE::ActorValue::kMagicka:
			_colorOverrides[a_actorHandle][BarType::kMagicka].SetOverride(a_colorType, a_color);
			break;
		case ConsoleRE::ActorValue::kStamina:
			_colorOverrides[a_actorHandle][BarType::kStamina].SetOverride(a_colorType, a_color);
			break;
		}

		_pendingColorChanges.emplace(a_actorHandle);
	}

	void TrueHUDMenu::OverrideSpecialBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, BarColorType a_colorType, uint32_t a_color)
	{
		_colorOverrides[a_actorHandle][BarType::kSpecial].SetOverride(a_colorType, a_color);

		_pendingColorChanges.emplace(a_actorHandle);
	}

	void TrueHUDMenu::RevertBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType)
	{
		switch (a_actorValue) {
		case ConsoleRE::ActorValue::kHealth:
			_colorOverrides[a_actorHandle][BarType::kHealth].RemoveOverride(a_colorType);
			if (_colorOverrides[a_actorHandle][BarType::kHealth].IsEmpty()) {
				_colorOverrides[a_actorHandle].erase(BarType::kHealth);
				if (_colorOverrides[a_actorHandle].empty()) {
					_colorOverrides.erase(a_actorHandle);
				}
			}
			break;
		case ConsoleRE::ActorValue::kMagicka:
			_colorOverrides[a_actorHandle][BarType::kMagicka].RemoveOverride(a_colorType);
			if (_colorOverrides[a_actorHandle][BarType::kMagicka].IsEmpty()) {
				_colorOverrides[a_actorHandle].erase(BarType::kMagicka);
				if (_colorOverrides[a_actorHandle].empty()) {
					_colorOverrides.erase(a_actorHandle);
				}
			}
			break;
		case ConsoleRE::ActorValue::kStamina:
			_colorOverrides[a_actorHandle][BarType::kStamina].RemoveOverride(a_colorType);
			if (_colorOverrides[a_actorHandle][BarType::kStamina].IsEmpty()) {
				_colorOverrides[a_actorHandle].erase(BarType::kStamina);
				if (_colorOverrides[a_actorHandle].empty()) {
					_colorOverrides.erase(a_actorHandle);
				}
			}
			break;
		}

		_pendingColorChanges.emplace(a_actorHandle);
	}

	void TrueHUDMenu::RevertSpecialBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, BarColorType a_colorType)
	{
		_colorOverrides[a_actorHandle][BarType::kSpecial].RemoveOverride(a_colorType);
		if (_colorOverrides[a_actorHandle][BarType::kSpecial].IsEmpty()) {
			_colorOverrides[a_actorHandle].erase(BarType::kSpecial);
			if (_colorOverrides[a_actorHandle].empty()) {
				_colorOverrides.erase(a_actorHandle);
			}
		}

		_pendingColorChanges.emplace(a_actorHandle);
	}

	uint32_t TrueHUDMenu::GetBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType) const
	{
		uint32_t retColor = 0xFFFFFF;

		bool bColorOverrideFound = false;

		BarType barType = BarType::kHealth;

		switch (a_actorValue) {
		case ConsoleRE::ActorValue::kHealth:
			barType = BarType::kHealth;
			break;
		case ConsoleRE::ActorValue::kMagicka:
			barType = BarType::kMagicka;
			break;
		case ConsoleRE::ActorValue::kStamina:
			barType = BarType::kStamina;
			break;
		}
		
		auto it = _colorOverrides.find(a_actorHandle);
		if (it != _colorOverrides.end()) {
			auto& bars = it->second;
			auto iter = bars.find(barType);
			if (iter != bars.end()) {
				auto& colors = iter->second;
				bColorOverrideFound = colors.GetColor(a_colorType, retColor);
			}
		}

		if (!bColorOverrideFound) {
			// no override found, return defaults
			retColor = GetDefaultColor(barType, a_colorType);
		}

		return retColor;
	}

	uint32_t TrueHUDMenu::GetSpecialBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, BarColorType a_colorType) const
	{
		uint32_t retColor = 0xFFFFFF;

		bool bColorOverrideFound = false;

		auto it = _colorOverrides.find(a_actorHandle);
		if (it != _colorOverrides.end()) {
			auto& bars = it->second;
			auto iter = bars.find(BarType::kSpecial);
			if (iter != bars.end()) {
				auto& colors = iter->second;
				bColorOverrideFound = colors.GetColor(a_colorType, retColor);
			}
		}

		if (!bColorOverrideFound) {
			// no override found, return defaults
			retColor = GetDefaultColor(BarType::kSpecial, a_colorType);
		}

		return retColor;
	}

	uint32_t TrueHUDMenu::GetDefaultColor(BarType a_barType, BarColorType a_barColorType) const
	{
		switch (a_barType) {
		case BarType::kHealth:
			switch (a_barColorType) {
			case BarColorType::BarColor:
				return Settings::uHealthColor;
			case BarColorType::PhantomColor:
				return Settings::uHealthPhantomColor;
			case BarColorType::BackgroundColor:
				return Settings::uHealthBackgroundColor;
			case BarColorType::PenaltyColor:
				return Settings::uHealthPenaltyColor;
			case BarColorType::FlashColor:
				return Settings::uHealthFlashColor;
			}
		case BarType::kMagicka:
			switch (a_barColorType) {
			case BarColorType::BarColor:
				return Settings::uMagickaColor;
			case BarColorType::PhantomColor:
				return Settings::uMagickaPhantomColor;
			case BarColorType::BackgroundColor:
				return Settings::uMagickaBackgroundColor;
			case BarColorType::PenaltyColor:
				return Settings::uMagickaPenaltyColor;
			case BarColorType::FlashColor:
				return Settings::uMagickaFlashColor;
			}
		case BarType::kStamina:
			switch (a_barColorType) {
			case BarColorType::BarColor:
				return Settings::uStaminaColor;
			case BarColorType::PhantomColor:
				return Settings::uStaminaPhantomColor;
			case BarColorType::BackgroundColor:
				return Settings::uStaminaBackgroundColor;
			case BarColorType::PenaltyColor:
				return Settings::uStaminaPenaltyColor;
			case BarColorType::FlashColor:
				return Settings::uStaminaFlashColor;
			}
		case BarType::kSpecial:
			switch (a_barColorType) {
			case BarColorType::BarColor:
				return Settings::uSpecialColor;
			case BarColorType::PhantomColor:
				return Settings::uSpecialPhantomColor;
			case BarColorType::BackgroundColor:
				return Settings::uSpecialBackgroundColor;
			case BarColorType::PenaltyColor:
				return Settings::uSpecialPenaltyColor;
			case BarColorType::FlashColor:
				return Settings::uSpecialFlashColor;
			}
		}

		return 0xFFFFFF;
	}

	void TrueHUDMenu::DrawLine(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		_linesToDraw.push_back(std::make_unique<DebugLine>(a_start, a_end, a_color, a_thickness, a_duration));
	}

	void TrueHUDMenu::DrawPoint(const ConsoleRE::NiPoint3& a_position, float a_size, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/)
	{
		_pointsToDraw.push_back(std::make_unique<DebugPoint>(a_position, a_color, a_size, a_duration));
	}

	void TrueHUDMenu::DrawArrow(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_size /*= 10.f*/, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		if (a_size <= 0.f) {
			a_size = 10.f;
		}

		DrawLine(a_start, a_end, a_duration, a_color, a_thickness);

		ConsoleRE::NiPoint3 direction = a_end - a_start;
		direction.Unitize();

		ConsoleRE::NiPoint3 upVector{ 0.f, 0.f, 1.f };
		ConsoleRE::NiPoint3 rightVector = direction.Cross(upVector);
		if (!Utils::IsNormalized(rightVector)) {
			Utils::FindBestAxisVectors(direction, upVector, rightVector);
		}

		ConsoleRE::NiPoint3 zeroVector;
		Utils::Matrix4 matrix;
		matrix.SetAxis(direction, rightVector, upVector, zeroVector);

		float arr = sqrtf(a_size);
		ConsoleRE::NiPoint3 arrowEnd1 = a_end + matrix.TransformPosition(ConsoleRE::NiPoint3(-arr, arr, 0.f));
		ConsoleRE::NiPoint3 arrowEnd2 = a_end + matrix.TransformPosition(ConsoleRE::NiPoint3(-arr, -arr, 0.f));
		DrawLine(a_end, arrowEnd1, a_duration, a_color, a_thickness);
		DrawLine(a_end, arrowEnd2, a_duration, a_color, a_thickness);
	}

	void TrueHUDMenu::DrawBox(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_extent, const ConsoleRE::NiQuaternion& a_rotation, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		ConsoleRE::NiPoint3 start = a_center + Utils::RotateVector({ a_extent.x, a_extent.y, a_extent.z }, a_rotation);
		ConsoleRE::NiPoint3 end = a_center + Utils::RotateVector({ a_extent.x, -a_extent.y, a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ a_extent.x, -a_extent.y, a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ -a_extent.x, -a_extent.y, a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ -a_extent.x, -a_extent.y, a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ -a_extent.x, a_extent.y, a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ -a_extent.x, a_extent.y, a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ a_extent.x, a_extent.y, a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ a_extent.x, a_extent.y, -a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ a_extent.x, -a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ a_extent.x, -a_extent.y, -a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ -a_extent.x, -a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ -a_extent.x, -a_extent.y, -a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ -a_extent.x, a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ -a_extent.x, a_extent.y, -a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ a_extent.x, a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ a_extent.x, a_extent.y, a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ a_extent.x, a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ a_extent.x, -a_extent.y, a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ a_extent.x, -a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ -a_extent.x, -a_extent.y, a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ -a_extent.x, -a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);

		start = a_center + Utils::RotateVector({ -a_extent.x, a_extent.y, a_extent.z }, a_rotation);
		end = a_center + Utils::RotateVector({ -a_extent.x, a_extent.y, -a_extent.z }, a_rotation);
		DrawLine(start, end, a_duration, a_color, a_thickness);
	}

	void TrueHUDMenu::DrawCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		const float angleDelta = 2.0f * PI / a_segments;
		ConsoleRE::NiPoint3 lastVertex = a_center + a_x * a_radius;

		for (uint32_t sideIndex = 0; sideIndex < a_segments; sideIndex++) {
			ConsoleRE::NiPoint3 vertex = a_center + (a_x * cosf(angleDelta * (sideIndex + 1)) + a_y * sinf(angleDelta * (sideIndex + 1))) * a_radius;
			DrawLine(lastVertex, vertex, a_duration, a_color, a_thickness);
			lastVertex = vertex;
		}
	}

	void TrueHUDMenu::DrawHalfCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		const float angleDelta = 2.0f * PI / a_segments;
		ConsoleRE::NiPoint3 lastVertex = a_center + a_x * a_radius;

		for (uint32_t sideIndex = 0; sideIndex < a_segments / 2; sideIndex++) {
			ConsoleRE::NiPoint3 vertex = a_center + (a_x * cosf(angleDelta * (sideIndex + 1)) + a_y * sinf(angleDelta * (sideIndex + 1))) * a_radius;
			DrawLine(lastVertex, vertex, a_duration, a_color, a_thickness);
			lastVertex = vertex;
		}
	}

	void TrueHUDMenu::DrawSphere(const ConsoleRE::NiPoint3& a_origin, float a_radius, uint32_t a_segments /*= 16*/, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		a_segments = max(a_segments, 4);

		ConsoleRE::NiPoint3 vertex1, vertex2, vertex3, vertex4;
		const float angleInc = 2.f * PI / a_segments;
		uint32_t numSegmentsY = a_segments, numSegmentsX;
		float latitude = angleInc, longitude;
		float sinY1 = 0.f, cosY1 = 1.f, sinY2, cosY2, sinX, cosX;
		
		while (numSegmentsY--) {
			sinY2 = sinf(latitude);
			cosY2 = cosf(latitude);

			vertex1 = ConsoleRE::NiPoint3{ sinY1, 0.f, cosY1 } * a_radius + a_origin;
			vertex3 = ConsoleRE::NiPoint3{ sinY2, 0.f, cosY2 } * a_radius + a_origin;
			longitude = angleInc;

			numSegmentsX = a_segments;
			while (numSegmentsX--) {
				sinX = sinf(longitude);
				cosX = cosf(longitude);

				vertex2 = ConsoleRE::NiPoint3{ (cosX * sinY1), (sinX * sinY1), cosY1 } * a_radius + a_origin;
				vertex4 = ConsoleRE::NiPoint3{ (cosX * sinY2), (sinX * sinY2), cosY2 } * a_radius + a_origin;

				DrawLine(vertex1, vertex2, a_duration, a_color, a_thickness);
				DrawLine(vertex1, vertex3, a_duration, a_color, a_thickness);

				vertex1 = vertex2;
				vertex3 = vertex4;
				longitude += angleInc;
			}
			sinY1 = sinY2;
			cosY1 = cosY2;
			latitude += angleInc;
		}
	}

	void TrueHUDMenu::DrawCylinder(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_radius, uint32_t a_segments, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		a_segments = max(a_segments, 4);

		ConsoleRE::NiPoint3 segment;
		ConsoleRE::NiPoint3 p1, p2, p3, p4;
		const float angleInc = (2 * PI) / a_segments;
		float angle = angleInc;

		ConsoleRE::NiPoint3 axis = a_end - a_start;
		axis.Unitize();
		if (axis.x == 0.f && axis.y == 0.f && axis.z == 0.f) {
			axis = { 0.f, 0.f, 1.f };
		}

		ConsoleRE::NiPoint3 perpendicular;
		ConsoleRE::NiPoint3 rightVector;

		Utils::FindBestAxisVectors(axis, perpendicular, rightVector);

		segment = Utils::RotateAngleAxis(perpendicular, 0.f, axis) * a_radius;
		p1 = segment + a_start;
		p3 = segment + a_end;

		while (a_segments--)
		{
			segment = Utils::RotateAngleAxis(perpendicular, angle, axis) * a_radius;
			p2 = segment + a_start;
			p4 = segment + a_end;

			DrawLine(p2, p4, a_duration, a_color, a_thickness);
			DrawLine(p1, p2, a_duration, a_color, a_thickness);
			DrawLine(p3, p4, a_duration, a_color, a_thickness);

			p1 = p2;
			p3 = p4;
			angle += angleInc;
		}
	}

	void TrueHUDMenu::DrawCone(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, uint32_t a_segments, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		a_segments = max(a_segments, 4);
		
		const float angle1 = a_angleHeight < 1e-4f ? 1e-4f : a_angleHeight < PI - 1e-4f ? a_angleHeight : PI - 1e-4f;
		const float angle2 = a_angleWidth < 1e-4f ? 1e-4f : a_angleWidth < PI - 1e-4f ? a_angleWidth : PI - 1e-4f;

		const float sinX2 = sinf(0.5f * angle1);
		const float sinY2 = sinf(0.5f * angle2);

		const float sinSqX2 = sinX2 * sinX2;
		const float sinSqY2 = sinY2 * sinY2;

		std::vector<ConsoleRE::NiPoint3> verts;
		verts.resize(a_segments);

		for (uint32_t i = 0; i < a_segments; i++) {
			const float fraction = (float)i / (float)(a_segments);
			const float thi = 2.f * PI * fraction;
			const float phi = atan2f(sinf(thi) * sinY2, cosf(thi) * sinX2);
			const float sinPhi = sinf(phi);
			const float cosPhi = cosf(phi);
			const float sinSqPhi = sinPhi * sinPhi;
			const float cosSqPhi = cosPhi * cosPhi;

			const float rSq = sinSqX2 * sinSqY2 / (sinSqX2 * sinSqPhi + sinSqY2 * cosSqPhi);
			const float r = sqrtf(rSq);
			const float sqr = sqrtf(1 - rSq);
			const float alpha = r * cosPhi;
			const float beta = r * sinPhi;

			verts[i].x = (1 - 2 * rSq);
			verts[i].y = 2 * sqr * alpha;
			verts[i].z = 2 * sqr * beta;
		}

		ConsoleRE::NiPoint3 yAxis, zAxis;
		ConsoleRE::NiPoint3 normalizedDir = a_direction;
		normalizedDir.Unitize();
		Utils::FindBestAxisVectors(normalizedDir, yAxis, zAxis);

		Utils::Matrix4 matrix(normalizedDir, yAxis, zAxis, a_origin);
		Utils::MatrixScale scale(a_length);
		matrix = scale * matrix;

		ConsoleRE::NiPoint3 currentPoint, prevPoint, firstPoint;
		for (uint32_t i = 0; i < a_segments; i++) {
			currentPoint = matrix.TransformPosition(verts[i]);
			ConsoleRE::NiPoint3 origin = matrix.GetOrigin();
			DrawLine(origin, currentPoint, a_duration, a_color, a_thickness);

			if (i > 0) {
				DrawLine(prevPoint, currentPoint, a_duration, a_color, a_thickness);
			} else {
				firstPoint = currentPoint;
			}

			prevPoint = currentPoint;
		}
		DrawLine(currentPoint, firstPoint, a_duration, a_color, a_thickness);
	}

	void TrueHUDMenu::DrawCapsule(const ConsoleRE::NiPoint3& a_origin, float a_halfHeight, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		constexpr int32_t collisionSides = 16;

		Utils::Matrix4 axis = Utils::MatrixQuatRotation(a_rotation);
		ConsoleRE::NiPoint3 xAxis = axis.GetScaledAxis(Utils::Matrix4::Axis::kX);
		ConsoleRE::NiPoint3 yAxis = axis.GetScaledAxis(Utils::Matrix4::Axis::kY);
		ConsoleRE::NiPoint3 zAxis = axis.GetScaledAxis(Utils::Matrix4::Axis::kZ);

		float halfAxis = max(a_halfHeight - a_radius, 1.f);
		ConsoleRE::NiPoint3 topEnd = a_origin + zAxis * halfAxis;
		ConsoleRE::NiPoint3 bottomEnd = a_origin - zAxis * halfAxis;

		// draw top and bottom circles
		DrawCircle(topEnd, xAxis, yAxis, a_radius, collisionSides, a_duration, a_color);
		DrawCircle(bottomEnd, xAxis, yAxis, a_radius, collisionSides, a_duration, a_color);

		// draw caps
		DrawHalfCircle(topEnd, yAxis, zAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);
		DrawHalfCircle(topEnd, xAxis, zAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);

		ConsoleRE::NiPoint3 negZAxis = -zAxis;

		DrawHalfCircle(bottomEnd, yAxis, negZAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);
		DrawHalfCircle(bottomEnd, xAxis, negZAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);

		// draw connected lines
		ConsoleRE::NiPoint3 start, end;
		start = topEnd + xAxis * a_radius;
		end = bottomEnd + xAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = topEnd - xAxis * a_radius;
		end = bottomEnd - xAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = topEnd + yAxis * a_radius;
		end = bottomEnd + yAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = topEnd - yAxis * a_radius;
		end = bottomEnd - yAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
	}

	void TrueHUDMenu::DrawCapsule(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		constexpr int32_t collisionSides = 16;

		auto rotatedVertexA = Utils::TransformVectorByMatrix(a_vertexA, Utils::QuaternionToMatrix(a_rotation));
		auto rotatedVertexB = Utils::TransformVectorByMatrix(a_vertexB, Utils::QuaternionToMatrix(a_rotation));

		Utils::Matrix4 axis = Utils::MatrixQuatRotation(a_rotation);
		ConsoleRE::NiPoint3 xAxis = axis.GetScaledAxis(Utils::Matrix4::Axis::kX);
		ConsoleRE::NiPoint3 yAxis = axis.GetScaledAxis(Utils::Matrix4::Axis::kY);
		ConsoleRE::NiPoint3 zAxis = axis.GetScaledAxis(Utils::Matrix4::Axis::kZ);

		ConsoleRE::NiPoint3 topEnd = a_origin + rotatedVertexA;
		ConsoleRE::NiPoint3 bottomEnd = a_origin + rotatedVertexB;

		// draw top and bottom circles
		DrawCircle(topEnd, xAxis, yAxis, a_radius, collisionSides, a_duration, a_color);
		DrawCircle(bottomEnd, xAxis, yAxis, a_radius, collisionSides, a_duration, a_color);

		// draw caps
		DrawHalfCircle(topEnd, yAxis, zAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);
		DrawHalfCircle(topEnd, xAxis, zAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);

		ConsoleRE::NiPoint3 negZAxis = -zAxis;

		DrawHalfCircle(bottomEnd, yAxis, negZAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);
		DrawHalfCircle(bottomEnd, xAxis, negZAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);

		// draw connected lines
		ConsoleRE::NiPoint3 start, end;
		start = topEnd + xAxis * a_radius;
		end = bottomEnd + xAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = topEnd - xAxis * a_radius;
		end = bottomEnd - xAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = topEnd + yAxis * a_radius;
		end = bottomEnd + yAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = topEnd - yAxis * a_radius;
		end = bottomEnd - yAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
	}

	void TrueHUDMenu::DrawCapsule(const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, float a_duration /*= 0.f*/, uint32_t a_color /*= 0xFF0000FF*/, float a_thickness /*= 1.f*/)
	{
		constexpr int32_t collisionSides = 16;

		ConsoleRE::NiPoint3 zAxis = a_vertexA - a_vertexB;
		zAxis.Unitize();

		// get other axis
		ConsoleRE::NiPoint3 upVector = (fabs(zAxis.z) < (1.f - 1.e-4f)) ? ConsoleRE::NiPoint3{ 0.f, 0.f, 1.f } : ConsoleRE::NiPoint3{ 1.f, 0.f, 0.f };
		ConsoleRE::NiPoint3 xAxis = upVector.UnitCross(zAxis);
		ConsoleRE::NiPoint3 yAxis = zAxis.Cross(xAxis);

		// draw top and bottom circles
		DrawCircle(a_vertexA, xAxis, yAxis, a_radius, collisionSides, a_duration, a_color);
		DrawCircle(a_vertexB, xAxis, yAxis, a_radius, collisionSides, a_duration, a_color);

		// draw caps
		DrawHalfCircle(a_vertexA, yAxis, zAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);
		DrawHalfCircle(a_vertexA, xAxis, zAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);

		ConsoleRE::NiPoint3 negZAxis = -zAxis;

		DrawHalfCircle(a_vertexB, yAxis, negZAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);
		DrawHalfCircle(a_vertexB, xAxis, negZAxis, a_radius, collisionSides, a_duration, a_color, a_thickness);

		// draw connected lines
		ConsoleRE::NiPoint3 start, end;
		start = a_vertexA + xAxis * a_radius;
		end = a_vertexB + xAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = a_vertexA - xAxis * a_radius;
		end = a_vertexB - xAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = a_vertexA + yAxis * a_radius;
		end = a_vertexB + yAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
		start = a_vertexA - yAxis * a_radius;
		end = a_vertexB - yAxis * a_radius;
		DrawLine(start, end, a_duration, a_color, a_thickness);
	}

	void TrueHUDMenu::SetMenuVisibilityMode(MenuVisibilityMode a_mode)
	{
		_menuVisibilityMode = a_mode;

		UpdateVisibility();
	}

	void TrueHUDMenu::SetCartMode(bool a_enable)
	{
		_bCartMode = a_enable;

		UpdateVisibility();
	}

	void TrueHUDMenu::UpdateSettings()
	{
		Locker locker(_lock);

		// Actor info bars
		for (auto& entry : _actorInfoBarMap) {
			auto& widget = entry.second;
			if (widget->_object.IsDisplayObject()) {
				widget->Initialize();
			}
		}

		// Boss bars
		for (auto& entry : _bossInfoBarMap) {
			auto& widget = entry.second;
			if (widget->_object.IsDisplayObject()) {
				widget->Initialize();
			}
		}

		// Player widget
		if (_playerWidget) {
			if (_playerWidget->_object.IsDisplayObject()) {
				_playerWidget->Initialize();
			}
		}

		// Recent loot widget
		if (_recentLoot) {
			if (_recentLoot->_object.IsDisplayObject()) {
				_recentLoot->Initialize();
			}
		}

		// Plugin widgets
		for (auto& pluginWidgets : _customWidgets) {
			for (auto& widgetTypes : pluginWidgets.second) {
				for (auto& entry : widgetTypes.second) {
					auto& widget = entry.second;

					if (widget->_object.IsDisplayObject()) {
						widget->Initialize();
					}
				}
			}
		}
	}

	void TrueHUDMenu::RemoveAllWidgets()
	{
		Locker locker(_lock);

		for (auto& entry : _actorInfoBarMap)
		{
			auto& widget = entry.second;
			if (widget->_object.IsDisplayObject()) {
				ConsoleRE::GFxValue arg;
				arg.SetNumber(widget->_widgetID);

				widget->Dispose();
				_view->Invoke("_root.TrueHUD.RemoveInfoBarWidget", nullptr, &arg, 1);
			}
		}
		_actorInfoBarMap.clear();

		for (auto& entry : _bossInfoBarMap) {
			auto& widget = entry.second;
			if (widget->_object.IsDisplayObject()) {
				ConsoleRE::GFxValue arg;
				arg.SetNumber(widget->_widgetID);

				widget->Dispose();
				_view->Invoke("_root.TrueHUD.RemoveBossInfoBarWidget", nullptr, &arg, 1);
			}
		}
		_bossInfoBarMap.clear();

		_bossQueue.clear();

		RemoveShoutIndicator();
		RemovePlayerWidget();
		RemoveRecentLootWidget();

		for (auto& entry : _floatingTextMap) {
			auto& widget = entry.second;
			if (widget->_object.IsDisplayObject()) {
				widget->Dispose();
			}
		}
		_floatingTextMap.clear();

		for (auto& pluginWidgets : _customWidgets) {
			for (auto& widgetTypes : pluginWidgets.second) {
				for (auto& entry : widgetTypes.second) {
					auto& widget = entry.second;

					if (widget->_object.IsDisplayObject()) {
						ConsoleRE::GFxValue args[3];
						args[0].SetNumber(pluginWidgets.first);
						args[1].SetNumber(widgetTypes.first);
						args[2].SetNumber(entry.first);

						widget->Dispose();
						_view->Invoke("_root.TrueHUD.RemoveCustomWidget", nullptr, args, 3);
					}
				}
				widgetTypes.second.clear();
			}
			pluginWidgets.second.clear();
		}
		_customWidgets.clear();

		_view->Invoke("_root.TrueHUD.RemoveAllCustomWidgets", nullptr, nullptr, 0);
	}

	void TrueHUDMenu::OnOpen()
	{
		_bIsOpen = true;
	}

	void TrueHUDMenu::OnClose()
	{
		RemoveAllWidgets();

		if (_bSubtitleYSaved) {
			auto hud = ConsoleRE::UI::GetSingleton()->GetMenu(ConsoleRE::HUDMenu::MENU_NAME);
			hud.get()->uiMovie->SetVariable("HUDMovieBaseInstance.SubtitleTextHolder._y", _savedSubtitleY);
		}		

		if (_bCompassAlphaSaved) {
			auto hud = ConsoleRE::UI::GetSingleton()->GetMenu(ConsoleRE::HUDMenu::MENU_NAME);
			hud.get()->uiMovie->SetVariable("HUDMovieBaseInstance.CompassShoutMeterHolder._alpha", _savedCompassAlpha);
		}

		_bIsOpen = false;
	}

	void TrueHUDMenu::ProcessDelegate(float a_deltaTime)
	{
		HUDHandler::GetSingleton()->Process(*this, a_deltaTime);
	}

	void TrueHUDMenu::Update(float a_deltaTime)
	{
		using WidgetState = WidgetBase::WidgetState;

		if (_menuVisibilityMode == MenuVisibilityMode::kHidden) {
			if (ConsoleRE::UI::GetSingleton()->GameIsPaused()) {
				return;
			}
		}

		if (Settings::bHideVanillaTargetBar || Settings::uBossBarModifyHUD == BossBarModifyHUD::kHideCompass && !_bossInfoBarMap.empty()) {
			auto hudPtr = ConsoleRE::UI::GetSingleton()->GetMenu(ConsoleRE::HUDMenu::MENU_NAME);
			if (hudPtr) {
				auto hud = hudPtr.get();
				if (hud && hud->uiMovie) {
					if (!_bVanillaEnemyHealthAlphaSaved) {
						hud->uiMovie->GetVariable(&_savedVanillaEnemyHealthAlpha, "HUDMovieBaseInstance.EnemyHealth_mc._alpha");
						_bVanillaEnemyHealthAlphaSaved = true;
					}

					hud->uiMovie->SetVariable("HUDMovieBaseInstance.EnemyHealth_mc._alpha", 0.f);
					_bVanillaEnemyHealthHidden = true;
				}				
			}
		} else if (_bVanillaEnemyHealthHidden) {
			auto hudPtr = ConsoleRE::UI::GetSingleton()->GetMenu(ConsoleRE::HUDMenu::MENU_NAME);
			if (hudPtr) {
				auto hud = hudPtr.get();
				if (hud && hud->uiMovie) {
					hud->uiMovie->SetVariable("HUDMovieBaseInstance.EnemyHealth_mc._alpha", _savedVanillaEnemyHealthAlpha);
					_bVanillaEnemyHealthHidden = false;
				}				
			}
		}

		if (Settings::bEnablePlayerWidget && !_playerWidget) {
			AddPlayerWidget();
		} else if (!Settings::bEnablePlayerWidget && _playerWidget) {
			RemovePlayerWidget();
		}

		if (Settings::bEnableRecentLoot && !_recentLoot) {
			AddRecentLootWidget();
		} else if (!Settings::bEnableRecentLoot && _recentLoot) {
			RemoveRecentLootWidget();
		}

		ConsoleRE::GFxValue depthArray;
		_view->CreateArray(&depthArray);

		// actor info bars
		for (auto widget_it = _actorInfoBarMap.begin(), next_widget_it = widget_it; widget_it != _actorInfoBarMap.end(); widget_it = next_widget_it) {
			++next_widget_it;
		
			auto& entry = *widget_it;
			auto& widget = entry.second;
				
			widget->ProcessDelegates();
			widget->Update(a_deltaTime);

			// remove widgets that need to be removed
			if (widget->_widgetState == WidgetState::kRemoved) {
				RemoveActorInfoBar(entry.first, WidgetRemovalMode::Immediate);
				continue;
			}

			// add to depths array
			AddToDepthsArray(widget, static_cast<uint32_t>(TrueHUDWidgetType::kInfoBar), depthArray);
		}
		
		// boss bars
		for (auto widget_it = _bossInfoBarMap.begin(), next_widget_it = widget_it; widget_it != _bossInfoBarMap.end(); widget_it = next_widget_it) {
			++next_widget_it;
		
			auto& entry = *widget_it;
			auto& widget = entry.second;
				
			widget->ProcessDelegates();
			widget->Update(a_deltaTime);

			// remove widgets that need to be removed
			if (widget->_widgetState == WidgetState::kRemoved) {
				RemoveBossInfoBar(entry.first, WidgetRemovalMode::Immediate);
				continue;
			}

			// add to depths array
			AddToDepthsArray(widget, static_cast<uint32_t>(TrueHUDWidgetType::kBossBar), depthArray);
		}
		
		if (_shoutIndicator) {
			_shoutIndicator->ProcessDelegates();
			_shoutIndicator->Update(a_deltaTime);

			AddToDepthsArray(_shoutIndicator, static_cast<uint32_t>(TrueHUDWidgetType::kStandaloneShoutIndicator), depthArray);
		}

		if (_playerWidget) {
			_playerWidget->ProcessDelegates();
			_playerWidget->Update(a_deltaTime);

			AddToDepthsArray(_playerWidget, static_cast<uint32_t>(TrueHUDWidgetType::kPlayerWidget), depthArray);
		}

		if (_recentLoot) {
			_recentLoot->ProcessDelegates();
			_recentLoot->Update(a_deltaTime);

			AddToDepthsArray(_recentLoot, static_cast<uint32_t>(TrueHUDWidgetType::kRecentLootWidget), depthArray);
		}

		// floating text
		for (auto widget_it = _floatingTextMap.begin(), next_widget_it = widget_it; widget_it != _floatingTextMap.end(); widget_it = next_widget_it) {
			++next_widget_it;

			auto& entry = *widget_it;
			auto& widget = entry.second;

			// remove widgets that need to be removed
			ConsoleRE::GFxValue::DisplayInfo displayInfo;
			if (!widget->_object.GetDisplayInfo(&displayInfo)) {
				_floatingTextMap.erase(widget_it);
				continue;
			}

			widget->ProcessDelegates();
			widget->Update(a_deltaTime);

			// add to depths array
			AddToDepthsArray(widget, static_cast<uint32_t>(TrueHUDWidgetType::kFloatingText), depthArray);
		}

		// custom widgets
		for (auto& pluginWidgets : _customWidgets) {
			ConsoleRE::GFxValue args[2];
			args[0].SetNumber(pluginWidgets.first);
			_view->CreateArray(&args[1]);
			for (auto& widgetTypes : pluginWidgets.second) {
				for (auto widget_it = widgetTypes.second.begin(), next_widget_it = widget_it; widget_it != widgetTypes.second.end(); widget_it = next_widget_it) {
					++next_widget_it;

					auto& entry = *widget_it;
					auto& widget = entry.second;

					widget->ProcessDelegates();
					widget->Update(a_deltaTime);

					// remove widgets that need to be removed
					if (widget->_widgetState == WidgetState::kRemoved) {
						RemoveWidget(pluginWidgets.first, widgetTypes.first, entry.first, WidgetRemovalMode::Immediate);
						continue;
					}

					// add to depths array
					AddToDepthsArray(widget, widgetTypes.first, args[1]);
				}
			}
			_view->Invoke("_root.TrueHUD.SortCustomWidgetDepths", nullptr, args, 2);
		}

		// sort widget depths
		_view->Invoke("_root.TrueHUD.SortDepths", nullptr, &depthArray, 1);

		UpdateColors();

		UpdateDebugDraw(a_deltaTime);
	}

	void TrueHUDMenu::CacheData()
	{
		if (_bCachedData) {
			return;
		}

		if (!_view) {
			return;
		}

		ConsoleRE::GRectF rect = _view->GetVisibleFrameRect();
		_screenRes.x = fabs(rect.left - rect.right);
		_screenRes.y = fabs(rect.top - rect.bottom);

		_bCachedData = true;
	}

	void TrueHUDMenu::UpdateColors()
	{
		auto playerHandle = ConsoleRE::PlayerCharacter::GetSingleton()->GetHandle();
		for (auto actorHandle : _pendingColorChanges) {
			if (actorHandle == playerHandle) {
				if (_playerWidget) {
					_playerWidget->RefreshColors();
					continue;
				}
			}

			// try the boss bars first
			auto it = _bossInfoBarMap.find(actorHandle);
			if (it != _bossInfoBarMap.end()) {
				auto& widget = it->second;
				widget->RefreshColors();
				continue;
			}

			// try normal info bars
			auto iter = _actorInfoBarMap.find(actorHandle);
			if (iter != _actorInfoBarMap.end()) {
				auto& widget = iter->second;
				widget->RefreshColors();
				continue;
			}
		}
		_pendingColorChanges.clear();
	}

	void TrueHUDMenu::UpdateVisibility()
	{
		if (_view) {
			if (_bCartMode) {
				_view->SetVisible(false);
			} else {
				switch (_menuVisibilityMode) {
				case MenuVisibilityMode::kHidden:
					_view->SetVisible(false);
					break;
				case MenuVisibilityMode::kPartial:
					{
						ConsoleRE::GFxValue arg;
						arg.SetNumber(1);
						_view->Invoke("_root.TrueHUD.SetVisibilityMode", nullptr, &arg, 1);
						_view->SetVisible(true);
						break;
					}
				case MenuVisibilityMode::kVisible:
					{
						ConsoleRE::GFxValue arg;
						arg.SetNumber(2);
						_view->Invoke("_root.TrueHUD.SetVisibilityMode", nullptr, &arg, 1);
						_view->SetVisible(true);
						break;
					}
				}
			}
		}
	}

	bool TrueHUDMenu::AddBossInfoBarWidget(ConsoleRE::ObjectRefHandle a_actorHandle)
	{
		Locker locker(_lock);

		RemoveActorInfoBar(a_actorHandle, WidgetRemovalMode::Immediate);

		auto widget = std::make_shared<BossInfoBar>(_view, a_actorHandle);

		auto iter = _bossInfoBarMap.emplace(a_actorHandle, widget);
		if (iter.second) {
			ConsoleRE::GFxValue arg;
			arg.SetNumber(widget->_widgetID);

			_view->Invoke("_root.TrueHUD.AddBossInfoBarWidget", &widget->_object, &arg, 1);
			if (!widget->_object.IsDisplayObject()) {
				_bossInfoBarMap.erase(iter.first);
				return false;
			}

			widget->SetIndex(GetNextBossBarIndex());
			RefreshBossBarIndexes(-1);
			widget->Initialize();
			return true;
		}

		return false;
	}

	int32_t TrueHUDMenu::GetNextBossBarIndex() const
	{
		return static_cast<int32_t>(_bossInfoBarMap.size() - 1);
	}

	void TrueHUDMenu::RefreshBossBarIndexes(int32_t a_removedIndex)
	{
		if (a_removedIndex != -1) {
			for (auto& widget : _bossInfoBarMap) {
				int32_t index = widget.second->GetIndex();
				if (index > a_removedIndex) {
					widget.second->SetIndex(index - 1);
				}
			}
		}

		switch (Settings::uBossBarModifyHUD) {
		case BossBarModifyHUD::kMoveSubtitles:
			{
				auto hud = ConsoleRE::UI::GetSingleton()->GetMenu(ConsoleRE::HUDMenu::MENU_NAME);

				if (!_bSubtitleYSaved) {
					hud.get()->uiMovie->GetVariable(&_savedSubtitleY, "HUDMovieBaseInstance.SubtitleTextHolder._y");
					_bSubtitleYSaved = true;
				}

				hud.get()->uiMovie->SetVariable("HUDMovieBaseInstance.SubtitleTextHolder._y", _savedSubtitleY.GetNumber() - Settings::fMultipleBossBarsOffset * _bossInfoBarMap.size());
				break;
			}

		case BossBarModifyHUD::kHideCompass:
			{
				auto hud = ConsoleRE::UI::GetSingleton()->GetMenu(ConsoleRE::HUDMenu::MENU_NAME);

				if (!_bCompassAlphaSaved) {
					hud.get()->uiMovie->GetVariable(&_savedCompassAlpha, "HUDMovieBaseInstance.CompassShoutMeterHolder._alpha");
					_bCompassAlphaSaved = true;
				}

				ConsoleRE::GFxValue zeroAlpha;
				zeroAlpha.SetNumber(0.f);
				hud.get()->uiMovie->SetVariable("HUDMovieBaseInstance.CompassShoutMeterHolder._alpha", _bossInfoBarMap.empty() ? _savedCompassAlpha : zeroAlpha);

				if (Settings::bDisplayStandaloneShoutWidgetWhenHidingCompass && (!Settings::bEnablePlayerWidget || Settings::uPlayerWidgetShoutIndicatorMode == PlayerWidgetDisplayMode::kNever)) {
					if (!_bossInfoBarMap.empty()) {
						AddShoutIndicator();
					} else {
						RemoveShoutIndicator();
					}
				}
				break;
			}
		}
	}

	void TrueHUDMenu::UpdateBossQueue()
	{
		if (_bossQueue.size() > 0 && _bossInfoBarMap.size() < Settings::uBossBarMaxCount)
		{
			auto boss = _bossQueue.begin();

			bool bSuccess = AddBossInfoBarWidget(*boss);

			if (bSuccess) {
				_bossQueue.pop_front();
			}
		}
	}

	void TrueHUDMenu::AddToDepthsArray(std::shared_ptr<WidgetBase> a_widget, uint32_t a_widgetType, ConsoleRE::GFxValue& a_array)
	{
		ConsoleRE::GFxValue data;
		_view->CreateObject(&data);

		ConsoleRE::GFxValue id;
		id.SetNumber(a_widget->_widgetID);
		ConsoleRE::GFxValue zIndex;
		zIndex.SetNumber(a_widget->_depth);
		ConsoleRE::GFxValue widgetType;
		widgetType.SetNumber(a_widgetType);

		data.SetMember("id", id);
		data.SetMember("zIndex", zIndex);
		data.SetMember("widgetType", widgetType);

		a_array.PushBack(data);
	}

	void TrueHUDMenu::UpdateDebugDraw(float a_deltaTime)
	{
		if (!_view) {
			return;
		}

		CacheData();
		ClearLines();

		for (int i = 0; i < _linesToDraw.size(); ++i) {
			auto line = _linesToDraw[i].get();

			DrawLine3D(line->start, line->end, line->color, line->thickness);
			line->duration -= a_deltaTime;
			if (line->duration < 0.f) {
				_linesToDraw.erase(_linesToDraw.begin() + i);
				--i;
				continue;
			}
		}

		for (int i = 0; i < _pointsToDraw.size(); ++i) {
			auto point = _pointsToDraw[i].get();

			DrawPoint3D(point->position, point->color, point->size);
			point->duration -= a_deltaTime;
			if (point->duration < 0.f) {
				_pointsToDraw.erase(_pointsToDraw.begin() + i);
				--i;
				continue;
			}
		}
	}

	void TrueHUDMenu::DrawLine2D(ConsoleRE::NiPoint2& a_start, ConsoleRE::NiPoint2& a_end, uint32_t a_color, float a_thickness)
	{
		// all parts of the line are offscreen - no need to draw
		if (!IsOnScreen(a_start, a_end)) {
			return;
		}

		ClampToScreen(a_start);
		ClampToScreen(a_end);

		uint32_t rgb = a_color >> 8;
		uint32_t alpha = a_color & 0x000000FF;

		ConsoleRE::GFxValue argsLineStyle[3]{ a_thickness, rgb, alpha };
		_view->Invoke("lineStyle", nullptr, argsLineStyle, 3);

		ConsoleRE::GFxValue argsStartPos[2]{ a_start.x, a_start.y };
		_view->Invoke("moveTo", nullptr, argsStartPos, 2);

		ConsoleRE::GFxValue argsEndPos[2]{ a_end.x, a_end.y };
		_view->Invoke("lineTo", nullptr, argsEndPos, 2);

		_view->Invoke("endFill", nullptr, nullptr, 0);

	}

	// https://www.oreilly.com/library/view/actionscript-cookbook/0596004907/ch04s06.html
	void TrueHUDMenu::DrawPoint2D(ConsoleRE::NiPoint2& a_position, uint32_t a_color, float a_size)
	{
		if (!IsOnScreen(a_position)) {
			return;
		}

		uint32_t rgb = a_color >> 8;
		uint32_t alpha = a_color & 0x000000FF;

		// The angle of each of the eight segments is 45 degrees (360 divided by 8), which
		// equals π/4 radians.
		constexpr float angleDelta = PI / 4;

		// Find the distance from the circle's center to the control points for the curves.
		float ctrlDist = a_size / cosf(angleDelta / 2.f);

		// Initialize the angle
		float angle = 0.f;

		ConsoleRE::GFxValue argsLineStyle[3]{ 0, 0, 0 };
		_view->Invoke("lineStyle", nullptr, argsLineStyle, 3);

		ConsoleRE::GFxValue argsFill[2]{ rgb, alpha };
		_view->Invoke("beginFill", nullptr, argsFill, 2);

		// Move to the starting point, one radius to the right of the circle's center.
		ConsoleRE::GFxValue argsStartPos[2]{ a_position.x + a_size, a_position.y };
		_view->Invoke("moveTo", nullptr, argsStartPos, 2);

		// Repeat eight times to create eight segments.
		for (int i = 0; i < 8; ++i) {
			// Increment the angle by angleDelta (π/4) to create the whole circle (2π).
			angle += angleDelta;

			// The control points are derived using sine and cosine.
			float rx = a_position.x + cosf(angle - (angleDelta / 2)) * (ctrlDist);
			float ry = a_position.y + sinf(angle - (angleDelta / 2)) * (ctrlDist);

			// The anchor points (end points of the curve) can be found similarly to the
			// control points.
			float ax = a_position.x + cosf(angle) * a_size;
			float ay = a_position.y + sinf(angle) * a_size;

			// Draw the segment.
			ConsoleRE::GFxValue argsCurveTo[4]{ rx, ry, ax, ay };
			_view->Invoke("curveTo", nullptr, argsCurveTo, 4);
		}

		_view->Invoke("endFill", nullptr, nullptr, 0);
	}

	void TrueHUDMenu::DrawLine3D(ConsoleRE::NiPoint3& a_start, ConsoleRE::NiPoint3& a_end, uint32_t a_color, float a_thickness)
	{
		if (Utils::IsBehindPlayerCamera(a_start) && Utils::IsBehindPlayerCamera(a_end)) {
			return;
		}

		ConsoleRE::NiPoint2 screenPosStart = WorldToScreen(a_start);
		ConsoleRE::NiPoint2 screenPosEnd = WorldToScreen(a_end);

		DrawLine2D(screenPosStart, screenPosEnd, a_color, a_thickness);
	}

	void TrueHUDMenu::DrawPoint3D(ConsoleRE::NiPoint3& a_position, uint32_t a_color, float a_size)
	{
		if (Utils::IsBehindPlayerCamera(a_position)) {
			return;
		}

		ConsoleRE::NiPoint2 screenPosition = WorldToScreen(a_position);

		DrawPoint2D(screenPosition, a_color, a_size);
	}

	void TrueHUDMenu::ClearLines()
	{
		_view->Invoke("clear", nullptr, nullptr, 0);
	}

	ConsoleRE::NiPoint2 TrueHUDMenu::WorldToScreen(const ConsoleRE::NiPoint3& a_worldPos) const
	{
		ConsoleRE::NiPoint2 screenPos;
		float depth;

		ConsoleRE::NiCamera::WorldPtToScreenPt3((float(*)[4])g_worldToCamMatrix, *g_viewPort, a_worldPos, screenPos.x, screenPos.y, depth, 1e-5f);
		ConsoleRE::GRectF rect = _view->GetVisibleFrameRect();

		screenPos.x = rect.left + (rect.right - rect.left) * screenPos.x;
		screenPos.y = 1.f - screenPos.y;
		screenPos.y = rect.top + (rect.bottom - rect.top) * screenPos.y;

		return screenPos;
	}

	void TrueHUDMenu::ClampToScreen(ConsoleRE::NiPoint2& a_point)
	{
		constexpr float maxOvershoot = 10000.f;

		if (a_point.x < 0.f) {
			float overshootX = fabs(a_point.x);
			if (overshootX > maxOvershoot) {
				a_point.x += overshootX - maxOvershoot;
			}
		} else if (a_point.x > _screenRes.x) {
			float overshootX = a_point.x - _screenRes.x;
			if (overshootX > maxOvershoot) {
				a_point.x -= overshootX - maxOvershoot;
			}
		}

		if (a_point.y < 0.f) {
			float overshootY = fabs(a_point.y);
			if (overshootY > maxOvershoot) {
				a_point.y += overshootY - maxOvershoot;
			}
		} else if (a_point.y > _screenRes.y) {
			float overshootY = a_point.y - _screenRes.y;
			if (overshootY > maxOvershoot) {
				a_point.y -= overshootY - maxOvershoot;
			}
		}
	}

	bool TrueHUDMenu::IsOnScreen(ConsoleRE::NiPoint2& a_start, ConsoleRE::NiPoint2& a_end) const
	{
		return IsOnScreen(a_start) || IsOnScreen(a_end);
	}

	bool TrueHUDMenu::IsOnScreen(ConsoleRE::NiPoint2& a_point) const
	{
		return (a_point.x <= _screenRes.x && a_point.x >= 0.0 && a_point.y <= _screenRes.y && a_point.y >= 0.0);
	}

	bool TrueHUDMenu::BarColorOverride::IsEmpty() const
	{
		return !bOverrideBarColor && !bOverridePhantomColor && !bOverrideBackgroundColor && !bOverridePenaltyColor && !bOverrideFlashColor;
	}

	bool TrueHUDMenu::BarColorOverride::GetColor(BarColorType a_colorType, uint32_t& a_outColor) const
	{
		switch (a_colorType) {
		case BarColorType::BarColor:
			if (bOverrideBarColor) {
				a_outColor = barColor;
				return true;
			}
			return false;
		case BarColorType::PhantomColor:
			if (bOverridePhantomColor) {
				a_outColor = phantomColor;
				return true;
			}
			return false;
		case BarColorType::BackgroundColor:
			if (bOverrideBackgroundColor) {
				a_outColor = backgroundColor;
				return true;
			}
			return false;
		case BarColorType::PenaltyColor:
			if (bOverridePenaltyColor) {
				a_outColor = penaltyColor;
				return true;
			}
			return false;
		case BarColorType::FlashColor:
			if (bOverrideFlashColor) {
				a_outColor = flashColor;
				return true;
			}
			return false;
		}
		return false;
	}

	void TrueHUDMenu::BarColorOverride::SetOverride(BarColorType a_colorType, uint32_t a_color)
	{
		switch (a_colorType) {
		case BarColorType::BarColor:
			barColor = a_color;
			bOverrideBarColor = true;
			break;
		case BarColorType::PhantomColor:
			phantomColor = a_color;
			bOverridePhantomColor = true;
			break;
		case BarColorType::BackgroundColor:
			backgroundColor = a_color;
			bOverrideBackgroundColor = true;
			break;
		case BarColorType::PenaltyColor:
			penaltyColor = a_color;
			bOverridePenaltyColor = true;
			break;
		case BarColorType::FlashColor:
			flashColor = a_color;
			bOverrideFlashColor = true;
			break;
		}
	}

	void TrueHUDMenu::BarColorOverride::RemoveOverride(BarColorType a_colorType)
	{
		switch (a_colorType) {
		case BarColorType::BarColor:
			bOverrideBarColor = false;
			break;
		case BarColorType::PhantomColor:
			bOverridePhantomColor = false;
			break;
		case BarColorType::BackgroundColor:
			bOverrideBackgroundColor = false;
			break;
		case BarColorType::PenaltyColor:
			bOverridePenaltyColor = false;
			break;
		case BarColorType::FlashColor:
			bOverrideFlashColor = false;
			break;
		}
	}

}
