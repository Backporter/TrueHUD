#include "HUDHandler.h"

#include "Offsets.h"
#include "Settings.h"
#include "Utils.h"

void HUDHandler::Register()
{
	auto scriptEventSourceHolder = ConsoleRE::ScriptEventSourceHolder::GetSingleton();
	
	//
	scriptEventSourceHolder->GetEventSource<ConsoleRE::TESCombatEvent>()->AddEventSink(HUDHandler::GetSingleton());
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::TESCombatEvent).name());
	
	//
	scriptEventSourceHolder->GetEventSource<ConsoleRE::TESDeathEvent>()->AddEventSink(HUDHandler::GetSingleton());
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::TESDeathEvent).name());
	
	//
	scriptEventSourceHolder->GetEventSource<ConsoleRE::TESEnterBleedoutEvent>()->AddEventSink(HUDHandler::GetSingleton());
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::TESEnterBleedoutEvent).name());
	
	//
	scriptEventSourceHolder->GetEventSource<ConsoleRE::TESHitEvent>()->AddEventSink(HUDHandler::GetSingleton());
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::TESHitEvent).name());
	
	auto ui = ConsoleRE::UI::GetSingleton();
	ui->AddEventSink<ConsoleRE::MenuOpenCloseEvent>(HUDHandler::GetSingleton());
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::MenuOpenCloseEvent).name());
}

// On combat start, add info/boss bar; on combat end, remove bar
HUDHandler::EventResult HUDHandler::ProcessEvent(const ConsoleRE::TESCombatEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESCombatEvent>*)
{
	using WidgetRemovalMode = TRUEHUD_API::WidgetRemovalMode;

	bool bInfoBarsEnabled = Settings::bEnableActorInfoBars && (Settings::uInfoBarDisplayHostiles > InfoBarsDisplayMode::kNever || Settings::uInfoBarDisplayTeammates > InfoBarsDisplayMode::kNever || Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever);

	if (bInfoBarsEnabled || Settings::bEnableBossBars) 
	{
		using CombatState = ConsoleRE::ACTOR_COMBAT_STATE;

		const auto isPlayerRef = [](auto&& a_ref) 
		{
			return a_ref && a_ref->IsPlayerRef();
		};

		auto eventActor = a_event->actor;
		auto eventTarget = a_event->targetActor;

		if (a_event && eventActor && eventTarget) 
		{
			bool bIsRelevant = eventTarget->IsPlayerRef();	
			if (!bIsRelevant && bInfoBarsEnabled) 
			{
				auto actor = eventActor->As<ConsoleRE::Actor>();
				if (Utils::IsPlayerTeammateOrSummon(actor)) 
				{
					bIsRelevant = true;
				}
			}

			if (bIsRelevant) 
			{
				auto actorHandle = eventActor->GetHandle();
				if (a_event->newState == (uint32_t)CombatState::kCombat) 
				{
					bool bHasBossBarAlready = HUDHandler::GetTrueHUDMenu()->HasBossInfoBar(actorHandle);
					if (bHasBossBarAlready) 
					{
						return EventResult::kContinue;
					}

					if (Settings::bEnableBossBars && CheckActorForBoss(actorHandle)) 
					{  // Add a boss bar

						HUDHandler::GetSingleton()->AddBossInfoBar(actorHandle);
					}
					else if (bInfoBarsEnabled)
					{  // Not a boss, add a normal info bar

						auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
						auto actor = eventActor->As<ConsoleRE::Actor>();

						if (actor->IsHostileToActor(playerCharacter)) 
						{
							if (Settings::uInfoBarDisplayHostiles > InfoBarsDisplayMode::kOnHit) 
							{
								HUDHandler::GetSingleton()->AddActorInfoBar(actorHandle);
							}
						}
						else if (Utils::IsPlayerTeammateOrSummon(actor)) 
						{
							if (Settings::uInfoBarDisplayTeammates > InfoBarsDisplayMode::kOnHit) 
							{
								HUDHandler::GetSingleton()->AddActorInfoBar(actorHandle);
							}
						} 
						else if (Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kOnHit) 
						{
							HUDHandler::GetSingleton()->AddActorInfoBar(actorHandle);
						}
					}
				} 
				else if (a_event->newState == (uint32_t)CombatState::kNone) 
				{  // Combat end, remove bar

					if (Settings::bEnableBossBars) 
					{
						HUDHandler::GetSingleton()->RemoveBossInfoBar(actorHandle, WidgetRemovalMode::Normal);
					}

					if (bInfoBarsEnabled) 
					{
						HUDHandler::GetSingleton()->RemoveActorInfoBar(actorHandle, WidgetRemovalMode::Normal);
					}
				}
			}
		}
	}


	return EventResult::kContinue;
}

// On death, remove info/boss bar
HUDHandler::EventResult HUDHandler::ProcessEvent(const ConsoleRE::TESDeathEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESDeathEvent>*)
{
	using WidgetRemovalMode = TRUEHUD_API::WidgetRemovalMode;

	if (a_event && a_event->actorDying) 
	{
		bool bInfoBarsEnabled = Settings::uInfoBarDisplayHostiles > InfoBarsDisplayMode::kNever || Settings::uInfoBarDisplayTeammates > InfoBarsDisplayMode::kNever || Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever;

		if (bInfoBarsEnabled || Settings::bEnableBossBars) 
		{
			auto actorHandle = a_event->actorDying->GetHandle();
			if (Settings::bEnableBossBars) 
			{
				HUDHandler::GetSingleton()->RemoveBossInfoBar(actorHandle, WidgetRemovalMode::Delayed);
			}

			if (bInfoBarsEnabled) 
			{
				HUDHandler::GetSingleton()->RemoveActorInfoBar(actorHandle, WidgetRemovalMode::Delayed);
			}
		}
	}

	return EventResult::kContinue;
}

// On bleedout, for essential targets
HUDHandler::EventResult HUDHandler::ProcessEvent(const ConsoleRE::TESEnterBleedoutEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESEnterBleedoutEvent>*)
{	
	using WidgetRemovalMode = TRUEHUD_API::WidgetRemovalMode;

	if (a_event && a_event->actor) {
		auto actor = a_event->actor->As<ConsoleRE::Actor>();
		if (actor && actor->IsEssential()) {
			bool bInfoBarsEnabled = Settings::uInfoBarDisplayHostiles > InfoBarsDisplayMode::kNever ||
			                        Settings::uInfoBarDisplayTeammates > InfoBarsDisplayMode::kNever ||
			                        Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever;

			if (bInfoBarsEnabled || Settings::bEnableBossBars) {
				auto actorHandle = a_event->actor->GetHandle();
				if (Settings::bEnableBossBars) {
					HUDHandler::GetSingleton()->RemoveBossInfoBar(actorHandle, WidgetRemovalMode::Delayed);
				}

				if (bInfoBarsEnabled) {
					HUDHandler::GetSingleton()->RemoveActorInfoBar(actorHandle, WidgetRemovalMode::Delayed);
				}
			}
		}
	}

	return EventResult::kContinue;
}


// On hit, add info/boss bar
HUDHandler::EventResult HUDHandler::ProcessEvent(const ConsoleRE::TESHitEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESHitEvent>*)
{
	bool bInfoBarsEnabled = Settings::bEnableActorInfoBars && (Settings::uInfoBarDisplayHostiles > InfoBarsDisplayMode::kNever || Settings::uInfoBarDisplayTeammates > InfoBarsDisplayMode::kNever || Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever);

	if (bInfoBarsEnabled || Settings::bEnableBossBars) 
	{
		if (a_event && a_event->cause && a_event->target) 
		{
			auto causeActorHandle = a_event->cause->GetHandle();
			auto targetActorHandle = a_event->target->GetHandle();

			if (causeActorHandle == targetActorHandle) 
			{
				return EventResult::kContinue;
			}

			if (causeActorHandle && targetActorHandle) 
			{
				auto causeActor = causeActorHandle.get()->As<ConsoleRE::Actor>();
				auto targetActor = targetActorHandle.get()->As<ConsoleRE::Actor>();

				if (causeActor && targetActor) 
				{
					if (causeActor->IsDead() || targetActor->IsDead()) 
					{
						return EventResult::kContinue;
					}

					bool bCausePlayer = causeActor->IsPlayerRef();
					bool bCausePlayerTeammate = Utils::IsPlayerTeammateOrSummon(causeActor);
					bool bTargetPlayer = targetActor->IsPlayerRef();
					bool bTargetPlayerTeammate = Utils::IsPlayerTeammateOrSummon(targetActor);

					if (bCausePlayer || bCausePlayerTeammate) 
					{
						bool bHasBossBarAlready = HUDHandler::GetTrueHUDMenu()->HasBossInfoBar(targetActorHandle);
						if (bHasBossBarAlready) 
						{
							return EventResult::kContinue;
						}

						if (Settings::bEnableBossBars && targetActor->IsInCombat() && CheckActorForBoss(targetActorHandle)) 
						{
							HUDHandler::GetSingleton()->AddBossInfoBar(targetActorHandle);
						} 
						else if (bInfoBarsEnabled) 
						{ 
							// Not a boss, add a normal info bar

							if (targetActor->IsHostileToActor(ConsoleRE::PlayerCharacter::GetSingleton())) 
							{

								if (Settings::uInfoBarDisplayHostiles > InfoBarsDisplayMode::kNever && targetActor->IsInCombat()) 
								{
									HUDHandler::GetSingleton()->AddActorInfoBar(targetActorHandle);	
								}
							}
							else if (!bTargetPlayerTeammate && Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever) 
							{  // don't show info bars on teammates when hit by player
								if (Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever && targetActor->IsInCombat()) 
								{
									HUDHandler::GetSingleton()->AddActorInfoBar(targetActorHandle);	
								}
							}
						}
					} 
					else if (bTargetPlayer || bTargetPlayerTeammate) 
					{
						bool bHasBossBarAlready = HUDHandler::GetTrueHUDMenu()->HasBossInfoBar(causeActorHandle);
						if (bHasBossBarAlready) 
						{
							return EventResult::kContinue;
						}

						if (Settings::bEnableBossBars && causeActor->IsInCombat() && CheckActorForBoss(causeActorHandle)) 
						{
							HUDHandler::GetSingleton()->AddBossInfoBar(causeActorHandle);
						}
						else if (bInfoBarsEnabled) 
						{  // Not a boss, add a normal info bar
							if (causeActor->IsHostileToActor(ConsoleRE::PlayerCharacter::GetSingleton())) 
							{
								if (Settings::uInfoBarDisplayHostiles > InfoBarsDisplayMode::kNever && causeActor->IsInCombat()) 
								{
									HUDHandler::GetSingleton()->AddActorInfoBar(causeActorHandle);
								}
							} 
							else if (!bCausePlayerTeammate && Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever) 
							{  // don't show info bars on teammates when they hit player
								
								if (Settings::uInfoBarDisplayOthers > InfoBarsDisplayMode::kNever && causeActor->IsInCombat()) 
								{
									HUDHandler::GetSingleton()->AddActorInfoBar(causeActorHandle);
								}
							}
						}
					}
				}
			}
		}
	}

	return EventResult::kContinue;
}

HUDHandler::EventResult HUDHandler::ProcessEvent(const ConsoleRE::MenuOpenCloseEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::MenuOpenCloseEvent>*)
{
	using ContextID = ConsoleRE::UserEvents::INPUT_CONTEXT_ID;

	// On HUD menu open/close - open/close the plugin's HUD menu
	if (a_event && !strcasecmp(a_event->menuName.data(), ConsoleRE::HUDMenu::MENU_NAME)) 
	{
		if (a_event->opening) 
		{
			HUDHandler::GetSingleton()->OpenTrueHUDMenu();
		} else 
		{
			HUDHandler::GetSingleton()->CloseTrueHUDMenu();
		}
	}

	// Hide the widgets when a menu is open
	auto controlMap = ConsoleRE::ControlMap::GetSingleton();
	if (controlMap) {
		auto& priorityStack = controlMap->contextPriorityStack;
		if (priorityStack.empty()) {
			HUDHandler::GetSingleton()->SetMenuVisibilityMode(MenuVisibilityMode::kHidden);
		} else if (priorityStack.back() == ContextID::kGameplay ||
				   priorityStack.back() == ContextID::kFavorites ||
				   priorityStack.back() == ContextID::kConsole) {
			HUDHandler::GetSingleton()->SetMenuVisibilityMode(MenuVisibilityMode::kVisible);
		} else if ((priorityStack.back() == ContextID::kCursor ||
					   priorityStack.back() == ContextID::kItemMenu ||
					   priorityStack.back() == ContextID::kMenuMode ||
					   priorityStack.back() == ContextID::kInventory) &&
				   (ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::DialogueMenu::MENU_NAME) ||
					   !Settings::bRecentLootHideInCraftingMenus && ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::CraftingMenu::MENU_NAME) ||
					   !Settings::bRecentLootHideInInventoryMenus && (ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::BarterMenu::MENU_NAME) ||
																		 ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::ContainerMenu::MENU_NAME) ||
																		 ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::GiftMenu::MENU_NAME) ||
																		 ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::InventoryMenu::MENU_NAME)))) {
			HUDHandler::GetSingleton()->SetMenuVisibilityMode(MenuVisibilityMode::kPartial);
		} else {
			HUDHandler::GetSingleton()->SetMenuVisibilityMode(MenuVisibilityMode::kHidden);
		}
	}

	return EventResult::kContinue;
}

void HUDHandler::OpenTrueHUDMenu()
{
	if (!IsTrueHUDMenuOpen()) {
		auto msgQ = ConsoleRE::UIMessageQueue::GetSingleton();
		if (msgQ) {
			msgQ->AddMessage(TrueHUDMenu::MenuName(), ConsoleRE::UI_MESSAGE_TYPE::kShow, nullptr);
		}
	}
}

void HUDHandler::CloseTrueHUDMenu()
{
	if (IsTrueHUDMenuOpen()) {
		auto msgQ = ConsoleRE::UIMessageQueue::GetSingleton();
		if (msgQ) {
			msgQ->AddMessage(TrueHUDMenu::MenuName(), ConsoleRE::UI_MESSAGE_TYPE::kHide, nullptr);
		}
	}
}

bool HUDHandler::IsTrueHUDMenuOpen()
{
	auto trueHUD = GetTrueHUDMenu();
	if (trueHUD) {
		return trueHUD->IsOpen();
	}
	return false;
}

ConsoleRE::GPtr<Scaleform::TrueHUDMenu> HUDHandler::GetTrueHUDMenu()
{
	auto ui = ConsoleRE::UI::GetSingleton();
	return ui ? ui->GetMenu<TrueHUDMenu>(TrueHUDMenu::MenuName()) : nullptr;
}

void HUDHandler::Update()
{

}

ConsoleRE::ObjectRefHandle HUDHandler::GetTarget() const
{
	return _currentTarget;
}

ConsoleRE::ObjectRefHandle HUDHandler::GetSoftTarget() const
{
	return _currentSoftTarget;
}

void HUDHandler::SetTarget(ConsoleRE::ObjectRefHandle a_actorHandle)
{
	if (a_actorHandle.native_handle() == 0x100000) {
		return;
	}

	AddHUDTask([a_actorHandle](TrueHUDMenu& a_menu) {
		a_menu.SetTarget(a_actorHandle);
	});

	_currentTarget = a_actorHandle;
}

void HUDHandler::SetSoftTarget(ConsoleRE::ObjectRefHandle a_actorHandle)
{
	if (a_actorHandle.native_handle() == 0x100000) {
		return;
	}

	AddHUDTask([a_actorHandle](TrueHUDMenu& a_menu) {
		a_menu.SetSoftTarget(a_actorHandle);
	});

	_currentSoftTarget = a_actorHandle;
}

bool HUDHandler::HasActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
{
	return GetTrueHUDMenu()->HasActorInfoBar(a_actorHandle);
}

void HUDHandler::AddActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
{
	if (!Settings::bEnableActorInfoBars) {
		return;
	}

	if (a_actorHandle.native_handle() == 0x100000) {
		return;
	}

	if (a_actorHandle) {
		AddHUDTask([a_actorHandle](TrueHUDMenu& a_menu) {
			a_menu.AddActorInfoBar(a_actorHandle);
		});
	}
}

void HUDHandler::RemoveActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode /*= kImmediate*/)
{
	if (a_actorHandle) {
		AddHUDTask([a_actorHandle, a_removalMode](TrueHUDMenu& a_menu) {
			a_menu.RemoveActorInfoBar(a_actorHandle, a_removalMode);
		});
	}
}

bool HUDHandler::HasBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
{
	return GetTrueHUDMenu()->HasBossInfoBar(a_actorHandle);
}

void HUDHandler::AddBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle)
{
	if (!Settings::bEnableBossBars) {
		return;
	}

	if (a_actorHandle.native_handle() == 0x100000) {
		return;
	}

	if (a_actorHandle) {
		AddHUDTask([a_actorHandle](TrueHUDMenu& a_menu) {
			a_menu.AddBossInfoBar(a_actorHandle);
		});
	}
}

void HUDHandler::RemoveBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode /*= kImmediate*/)
{
	if (a_actorHandle) {
		AddHUDTask([a_actorHandle, a_removalMode](TrueHUDMenu& a_menu) {
			a_menu.RemoveBossInfoBar(a_actorHandle, a_removalMode);
		});
	}
}

void HUDHandler::AddShoutIndicator()
{
	AddHUDTask([](TrueHUDMenu& a_menu) {
		a_menu.AddShoutIndicator();
	});
}

void HUDHandler::RemoveShoutIndicator()
{
	AddHUDTask([](TrueHUDMenu& a_menu) {
		a_menu.RemoveShoutIndicator();
	});
}

void HUDHandler::AddPlayerWidget()
{
	if (!Settings::bEnablePlayerWidget) {
		return;
	}

	AddHUDTask([](TrueHUDMenu& a_menu) {
		a_menu.AddPlayerWidget();
	});
}

void HUDHandler::RemovePlayerWidget()
{
	AddHUDTask([](TrueHUDMenu& a_menu) {
		a_menu.RemovePlayerWidget();
	});
}

void HUDHandler::UpdatePlayerWidgetChargeMeters(float a_percent, bool a_bForce, bool a_bLeftHand, bool a_bShow)
{
	if (!Settings::bEnablePlayerWidget || !Settings::bPlayerWidgetDisplayEnchantmentChargeMeter) 
	{
		return;
	}

	AddHUDTask([a_percent, a_bForce, a_bLeftHand, a_bShow](TrueHUDMenu& a_menu) 
	{
		a_menu.UpdatePlayerWidgetChargeMeters(a_percent, a_bForce, a_bLeftHand, a_bShow);
	});
}

void HUDHandler::AddRecentLootWidget()
{
	if (!Settings::bEnableRecentLoot) {
		return;
	}

	AddHUDTask([](TrueHUDMenu& a_menu) {
		a_menu.AddRecentLootWidget();
	});
}

void HUDHandler::RemoveRecentLootWidget()
{
	AddHUDTask([](TrueHUDMenu& a_menu) {
		a_menu.RemoveRecentLootWidget();
	});
}

void HUDHandler::AddRecentLootMessage(ConsoleRE::TESBoundObject* a_object, const char* a_name, uint32_t a_count)
{
	if (!Settings::bEnableRecentLoot) 
	{
		return;
	}

	if (strlen(a_name) == 0 || !strcasecmp(a_name, "") || !strcasecmp(a_name, " "))
	{
		return;
	}

	// Don't show message when those menus are open
	if (Settings::bRecentLootHideInInventoryMenus &&
		(ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::BarterMenu::MENU_NAME) ||
			ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::ContainerMenu::MENU_NAME) ||
			ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::GiftMenu::MENU_NAME))) 
	{
		return;
	}

	if (Settings::bRecentLootHideInCraftingMenus &&
		ConsoleRE::UI::GetSingleton()->IsMenuOpen(ConsoleRE::CraftingMenu::MENU_NAME)) 
	{
		return;
	}

	AddHUDTask([a_object, a_name, a_count](TrueHUDMenu& a_menu) 
	{
		a_menu.AddRecentLootMessage(a_object, a_name, a_count);
	});
}

void HUDHandler::AddFloatingWorldTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint3 a_worldPosition)
{
	if (!Settings::bEnableFloatingText) {
		return;
	}

	AddHUDTask([a_text, a_color, a_duration, a_bSpecial, a_worldPosition](TrueHUDMenu& a_menu) {
		a_menu.AddFloatingWorldTextWidget(a_text, a_color, a_duration, a_bSpecial, a_worldPosition);
	});
}

void HUDHandler::AddFloatingScreenTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint2 a_screenPosition)
{
	if (!Settings::bEnableFloatingText) {
		return;
	}

	AddHUDTask([a_text, a_color, a_duration, a_bSpecial, a_screenPosition](TrueHUDMenu& a_menu) {
		a_menu.AddFloatingScreenTextWidget(a_text, a_color, a_duration, a_bSpecial, a_screenPosition);
	});
}

void HUDHandler::AddStackingDamageWorldTextWidget(ConsoleRE::ObjectRefHandle a_refHandle, float a_damage)
{
	if (!Settings::bEnableFloatingText) {
		return;
	}

	auto it = _stackingDamage.find(a_refHandle);
	if (it != _stackingDamage.end()) {
		it->second.damage += a_damage;
	} else {
		_stackingDamage.emplace(a_refHandle, a_damage);
	}
}

void HUDHandler::OverrideBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType, uint32_t a_color)
{
	AddHUDTask([a_actorHandle, a_actorValue, a_colorType, a_color](TrueHUDMenu& a_menu) {
		a_menu.OverrideBarColor(a_actorHandle, a_actorValue, a_colorType, a_color);
	});
}

void HUDHandler::OverrideSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType, uint32_t a_color)
{
	AddHUDTask([a_actorHandle, a_colorType, a_color](TrueHUDMenu& a_menu) {
		a_menu.OverrideSpecialBarColor(a_actorHandle, a_colorType, a_color);
	});
}

void HUDHandler::RevertBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType)
{
	AddHUDTask([a_actorHandle, a_actorValue, a_colorType](TrueHUDMenu& a_menu) {
		a_menu.RevertBarColor(a_actorHandle, a_actorValue, a_colorType);
	});
}

void HUDHandler::RevertSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType)
{
	AddHUDTask([a_actorHandle, a_colorType](TrueHUDMenu& a_menu) {
		a_menu.RevertSpecialBarColor(a_actorHandle, a_colorType);
	});
}

void HUDHandler::LoadCustomWidgets(size_t a_pluginHandle, const char* a_filePath, APIResultCallback&& a_successCallback)
{
	AddHUDTask([a_pluginHandle, a_filePath, a_successCallback = std::forward<APIResultCallback>(a_successCallback)](TrueHUDMenu& a_menu) mutable {
		a_menu.LoadCustomWidgets(a_pluginHandle, a_filePath, std::move(a_successCallback));
	});
}

void HUDHandler::RegisterNewWidgetType(size_t a_pluginHandle, uint32_t a_widgetType)
{
	AddHUDTask([a_pluginHandle, a_widgetType](TrueHUDMenu& a_menu) {
		a_menu.RegisterNewWidgetType(a_pluginHandle, a_widgetType);
	});
}

void HUDHandler::AddWidget(size_t a_pluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, const char* a_symbolIdentifier, std::shared_ptr<WidgetBase> a_widget)
{
	AddHUDTask([a_pluginHandle, a_widgetType, a_widgetID, a_symbolIdentifier, a_widget](TrueHUDMenu& a_menu) {
		a_menu.AddWidget(a_pluginHandle, a_widgetType, a_widgetID, a_symbolIdentifier, a_widget);
	});
}

void HUDHandler::RemoveWidget(size_t a_pluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, WidgetRemovalMode a_removalMode)
{
	AddHUDTask([a_pluginHandle, a_widgetType, a_widgetID, a_removalMode](TrueHUDMenu& a_menu) mutable {
		a_menu.RemoveWidget(a_pluginHandle, a_widgetType, a_widgetID, a_removalMode);
	});
}

void HUDHandler::FlashActorValue(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, bool a_bLong)
{
	AddHUDTask([a_actorHandle, a_actorValue, a_bLong](TrueHUDMenu& a_menu) {
		a_menu.FlashActorValue(a_actorHandle, a_actorValue, a_bLong);
	});
}

void HUDHandler::FlashActorSpecialBar(ConsoleRE::ObjectRefHandle a_actorHandle, bool a_bLong)
{
	AddHUDTask([a_actorHandle, a_bLong](TrueHUDMenu& a_menu) {
		a_menu.FlashActorSpecialBar(a_actorHandle, a_bLong);
	});
}

void HUDHandler::DrawLine(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_start, a_end, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawLine(a_start, a_end, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawPoint(const ConsoleRE::NiPoint3& a_position, float a_size, float a_duration, uint32_t a_color)
{
	AddHUDTask([a_position, a_size, a_duration, a_color](TrueHUDMenu& a_menu) {
		a_menu.DrawPoint(a_position, a_size, a_duration, a_color);
	});
}

void HUDHandler::DrawArrow(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_size, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_start, a_end, a_size, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawArrow(a_start, a_end, a_size, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawBox(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_extent, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_center, a_extent, a_rotation, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawBox(a_center, a_extent, a_rotation, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawCircle(a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawHalfCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawHalfCircle(a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawSphere(const ConsoleRE::NiPoint3& a_origin, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_origin, a_radius, a_segments, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawSphere(a_origin, a_radius, a_segments, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawCylinder(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_start, a_end, a_radius, a_segments, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawCylinder(a_start, a_end, a_radius, a_segments, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawCone(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_origin, a_direction, a_length, a_angleWidth, a_angleHeight, a_segments, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawCone(a_origin, a_direction, a_length, a_angleWidth, a_angleHeight, a_segments, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawCapsule(const ConsoleRE::NiPoint3& a_origin, float a_halfHeight, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_origin, a_halfHeight, a_radius, a_rotation, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawCapsule(a_origin, a_halfHeight, a_radius, a_rotation, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::DrawCapsule(const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, float a_duration, uint32_t a_color, float a_thickness)
{
	AddHUDTask([a_vertexA, a_vertexB, a_radius, a_duration, a_color, a_thickness](TrueHUDMenu& a_menu) {
		a_menu.DrawCapsule(a_vertexA, a_vertexB, a_radius, a_duration, a_color, a_thickness);
	});
}

void HUDHandler::SetMenuVisibilityMode(MenuVisibilityMode a_mode)
{
	AddHUDTask([a_mode](TrueHUDMenu& a_menu) {
		a_menu.SetMenuVisibilityMode(a_mode);
	});
}

void HUDHandler::SetCartMode(bool a_enable)
{
	AddHUDTask([a_enable](TrueHUDMenu& a_menu) {
		a_menu.SetCartMode(a_enable);
	});
}

void HUDHandler::RemoveAllWidgets()
{
	AddHUDTask([](TrueHUDMenu& a_menu) {
		a_menu.RemoveAllWidgets();
	});
}

bool HUDHandler::CheckActorForBoss(ConsoleRE::ObjectRefHandle a_refHandle)
{
	if (!a_refHandle) {
		return false;
	}

	auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
	auto actor = a_refHandle.get()->As<ConsoleRE::Actor>();
	if (actor && playerCharacter && (actor != playerCharacter)) {
		// Check whether the target is even alive or hostile first
		if (actor->IsDead() || (actor->AsActorState()->IsBleedingOut() && actor->IsEssential()) || !actor->IsHostileToActor(playerCharacter)) {
			return false;
		}

		auto actorBase = actor->GetActorBase();
		ConsoleRE::TESActorBase* originalBase = nullptr;

		auto extraLeveledCreature = static_cast<ConsoleRE::ExtraLeveledCreature*>(actor->extraList.GetByType(ConsoleRE::ExtraDataType::kLeveledCreature));
		if (extraLeveledCreature) {
			originalBase = extraLeveledCreature->originalBase;
		}

		// Check blacklist
		if (Settings::bossNPCBlacklist.find(actorBase) != Settings::bossNPCBlacklist.end() || (originalBase && Settings::bossNPCBlacklist.find(originalBase) != Settings::bossNPCBlacklist.end()))
		{
			return false;
		}

		// Check race
		if (Settings::bossRaces.find(actor->GetRace()) != Settings::bossRaces.end())
		{
			return true;
		}

		// Check NPC
		if (Settings::bossNPCs.find(actorBase) != Settings::bossNPCs.end() || (originalBase && Settings::bossNPCs.find(originalBase) != Settings::bossNPCs.end()))
		{
			return true;
		}

		// Check current loc refs
		if (auto currentLocation = playerCharacter->currentLocation) 
		{
			for (auto& ref : currentLocation->specialRefs) 
			{
				if (ref.type && Settings::bossLocRefTypes.find(ref.type) != Settings::bossLocRefTypes.end() && ref.refData.refID == actor->formID)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void HUDHandler::OnPreLoadGame()
{
	//RemoveAllWidgets();
	//_bossTargets.clear();
}

void HUDHandler::OnSettingsUpdated()
{
	if (IsTrueHUDMenuOpen()) {
		AddHUDTask([](TrueHUDMenu& a_menu) 
		{
			xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Updating settings...");
			a_menu.UpdateSettings();
			xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");
		});
	}
}

void HUDHandler::Initialize()
{
	
}

void HUDHandler::Process(TrueHUDMenu& a_menu, float a_deltaTime)
{
	while (!_taskQueue.empty()) {
		auto& task = _taskQueue.front();
		task(a_menu);
		_taskQueue.pop();
	}

	for (auto it = _stackingDamage.begin(), next_it = it; it != _stackingDamage.end(); it = next_it) {
		++next_it;
		DamageStack& damageStack = it->second;
		damageStack.timeElapsed += a_deltaTime;
		if (damageStack.timeElapsed > _stackingPeriodDuration) {
			if (auto actor = it->first.get().get()) {
				HUDHandler::GetSingleton()->AddFloatingWorldTextWidget(std::to_string(static_cast<int32_t>(ceil(damageStack.damage))), 0xFFFFFF, 2.f, false, actor->GetLookingAtLocation());
			}
			_stackingDamage.erase(it);
		}
	}
}

HUDHandler::HUDHandler() :
	_lock()
{}

void HUDHandler::AddHUDTask(HUDTask a_task)
{
	OpenTrueHUDMenu();
	Locker locker(_lock);
	_taskQueue.push(std::move(a_task));
}
