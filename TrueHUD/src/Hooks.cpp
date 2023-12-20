#include "Hooks.h"
#include "Settings.h"
#include "Offsets.h"
#include "HUDHandler.h"
#include "ModAPI.h"

#include "../../../OrbisUtil/Third-Party/herumi/xbayk/6.00/xbyak.h"
// #include <xbyak/xbyak.h>

namespace Hooks
{
	void Install()
	{
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Hooking...");

		HUDHook::Hook();
		//DamageHook::Hook();

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");
	}

	bool HUDHook::ProcessMessage(uintptr_t a_enemyHealth, ConsoleRE::UIMessage* a_uiMessage)
	{
		bool bReturn = _ProcessMessage(a_enemyHealth, a_uiMessage);

		// if no other plugin controls targets, set the vanilla enemy target as the soft target
		auto trueHUDInterface = Messaging::TrueHUDInterface::GetSingleton();
		if (!trueHUDInterface->IsTargetControlTaken()) {
			ConsoleRE::ActorHandle actorHandle = *(ConsoleRE::ActorHandle*)(a_enemyHealth + 0x28);

			if (actorHandle) {
				HUDHandler::GetSingleton()->SetSoftTarget(actorHandle);
			} else {
				HUDHandler::GetSingleton()->SetSoftTarget(ConsoleRE::ActorHandle());
			}
		}

		return bReturn;
	}

	void HUDHook::ProcessButton(ConsoleRE::SprintHandler* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		_ProcessButton(a_this, a_event, a_data);

		if (Settings::bPlayerWidgetDisplayMountStamina) {
			ConsoleRE::ActorPtr playerMount = nullptr;
			bool bPlayerHasMount = ConsoleRE::PlayerCharacter::GetSingleton()->GetMount(playerMount);
			if (bPlayerHasMount && playerMount->AsActorValueOwner()->GetActorValue(ConsoleRE::ActorValue::kStamina) <= 0.f) {
				FlashHUDMenuMeter(ConsoleRE::ActorValue::kStamina);
			}
		}
	}

	void HUDHook::AddObjectToContainer(ConsoleRE::Actor* a_this, ConsoleRE::TESBoundObject* a_object, ConsoleRE::ExtraDataList* a_extraList, int32_t a_count, ConsoleRE::TESObjectREFR* a_fromRefr)
	{
		_AddObjectToContainer(a_this, a_object, a_extraList, a_count, a_fromRefr);

		if (Settings::bEnableRecentLoot && a_object->GetPlayable()) {
			HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count);
		}
	}

	void HUDHook::PickUpObject(ConsoleRE::Actor* a_this, ConsoleRE::TESObjectREFR* a_object, uint32_t a_count, bool a_arg3, bool a_playSound)
	{
		_PickUpObject(a_this, a_object, a_count, a_arg3, a_playSound);

		if (Settings::bEnableRecentLoot && a_object->GetPlayable()) {
			HUDHandler::GetSingleton()->AddRecentLootMessage(a_object->GetBaseObject(), a_object->GetDisplayFullName(), a_count);	
		}
	}

	void HUDHook::AddMessage_Flash(ConsoleRE::UIMessageQueue* a_this, const ConsoleRE::BSFixedString& a_menuName, ConsoleRE::UI_MESSAGE_TYPE a_type, ConsoleRE::HUDData* a_data)
	{
		_AddMessage_Flash(a_this, a_menuName, a_type, a_data);

		ConsoleRE::ActorValue actorValue = static_cast<ConsoleRE::ActorValue>(a_data->discovery);
		if (actorValue == ConsoleRE::ActorValue::kVoiceRate || actorValue == ConsoleRE::ActorValue::kStamina || actorValue == ConsoleRE::ActorValue::kMagicka || actorValue == ConsoleRE::ActorValue::kHealth) {
			
			HUDHandler::GetSingleton()->FlashActorValue(ConsoleRE::PlayerCharacter::GetSingleton()->GetHandle(), actorValue, true);
		}
	}

	void HUDHook::AddMessage_SetHUDCartMode(ConsoleRE::UIMessageQueue* a_this, const ConsoleRE::BSFixedString& a_menuName, ConsoleRE::UI_MESSAGE_TYPE a_type, ConsoleRE::HUDData* a_data)
	{
		_AddMessage_SetHUDCartMode(a_this, a_menuName, a_type, a_data);

		if (a_data && a_data->text == "CartMode") 
		{
			HUDHandler::GetSingleton()->SetCartMode(a_data->unk38);
		}
	}

	bool HUDHook::Invoke_ChargeMeter1(ConsoleRE::GFxValue::ObjectInterface* a_this, void* a_data, ConsoleRE::GFxValue* a_result, const char* a_name, const ConsoleRE::GFxValue* a_args, size_t a_numArgs, bool a_isDObj)
	{
		bool bReturn = _Invoke_ChargeMeter1(a_this, a_data, a_result, a_name, a_args, a_numArgs, a_isDObj);

		if (Settings::bEnablePlayerWidget && Settings::bPlayerWidgetDisplayEnchantmentChargeMeter) {
			HUDHandler::GetSingleton()->UpdatePlayerWidgetChargeMeters(static_cast<float>(a_args[0].GetNumber()), a_args[1].GetBool(), a_args[2].GetBool(), a_args[3].GetBool());
		}		

		return bReturn;
	}

	bool HUDHook::Invoke_ChargeMeter2(ConsoleRE::GFxValue::ObjectInterface* a_this, void* a_data, ConsoleRE::GFxValue* a_result, const char* a_name, const ConsoleRE::GFxValue* a_args, size_t a_numArgs, bool a_isDObj)
	{
		bool bReturn = _Invoke_ChargeMeter2(a_this, a_data, a_result, a_name, a_args, a_numArgs, a_isDObj);

		if (Settings::bEnablePlayerWidget && Settings::bPlayerWidgetDisplayEnchantmentChargeMeter) {
			HUDHandler::GetSingleton()->UpdatePlayerWidgetChargeMeters(static_cast<float>(a_args[0].GetNumber()), a_args[1].GetBool(), a_args[2].GetBool(), a_args[3].GetBool());
		}		

		return bReturn;
	}

	void HUDHook::AddItem_AddItemFunctor(ConsoleRE::TESObjectREFR* a_this, ConsoleRE::TESObjectREFR* a_object, int32_t a_count, bool a4, bool a5)
	{
		_AddItem_AddItemFunctor(a_this, a_object, a_count, a4, a5);

		//static auto GetIsSilent = []() -> bool {
		//	struct GetIsSilent : Xbyak::CodeGenerator
		//	{
		//		GetIsSilent()
		//		{
		//			mov(rax, r12b); // r12 has the bIsSilent bool from AddItemFunctor
		//			ret();
		//		}
		//	} getIsSilent;

		//	return getIsSilent.getCode<bool(*)()>()();
		//};

		//bool bSilent = GetIsSilent();

		if (Settings::bEnableRecentLoot && a_this->IsPlayerRef() /*&& !bSilent*/ && a_object->GetPlayable()) 
		{
			HUDHandler::GetSingleton()->AddRecentLootMessage(a_object->GetBaseObject(), a_object->GetDisplayFullName(), a_count);
		}
	}

	void HUDHook::PlayPickupSoundAndMessage_AddItemFunctor(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5)
	{
		if (Settings::bEnableRecentLoot) {
			if (a_object->GetPlayable()) {
				//HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count); // no need as it already triggers the hook placed at PlayerCharacter::AddObjectToContainer
				if (Settings::bRecentLootHideVanillaMessage) {  // Skip original call
					if (a4) {  // Play pickup sound
						ConsoleRE::PlayerCharacter::GetSingleton()->PlayPickUpSound(a_object, a3, 0);
					}
					return;
				}
			}
		}

		_PlayPickupSoundAndMessage_AddItemFunctor(a_object, a_count, a3, a4, a5);
	}

	void HUDHook::PlayPickupSoundAndMessage_ActivateFlora1(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5)
	{
		if (Settings::bEnableRecentLoot) {
			if (a_object->GetPlayable()) {
				//HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count); // no need as it already triggers the hook placed at PlayerCharacter::AddObjectToContainer
				if (Settings::bRecentLootHideVanillaMessage) {  // Skip original call
					if (a4) {  // Play pickup sound
						ConsoleRE::PlayerCharacter::GetSingleton()->PlayPickUpSound(a_object, a3, 0);
					}
					return;
				}
			}
		}

		_PlayPickupSoundAndMessage_ActivateFlora1(a_object, a_count, a3, a4, a5);
	}

	void HUDHook::PlayPickupSoundAndMessage_ActivateFlora2(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5)
	{
		if (Settings::bEnableRecentLoot) {
			if (a_object->GetPlayable()) {
				HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count);
				if (Settings::bRecentLootHideVanillaMessage) {  // Skip original call
					if (a4) {  // Play pickup sound
						ConsoleRE::PlayerCharacter::GetSingleton()->PlayPickUpSound(a_object, a3, 0);
					}
					return;
				}
			}
		}

		_PlayPickupSoundAndMessage_ActivateFlora2(a_object, a_count, a3, a4, a5);
	}

	void HUDHook::PlayPickupSoundAndMessage_ConstructibleObjectMenu(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5)
	{
		if (Settings::bEnableRecentLoot && !Settings::bRecentLootHideInCraftingMenus) {
			if (a_object->GetPlayable()) {
				//HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count); // no need as it already triggers the hook placed at PlayerCharacter::AddObjectToContainer
				if (Settings::bRecentLootHideVanillaMessage) {  // Skip original call
					if (a4) {                                   // Play pickup sound
						ConsoleRE::PlayerCharacter::GetSingleton()->PlayPickUpSound(a_object, a3, 0);
					}
					return;
				}
			}
		}

		_PlayPickupSoundAndMessage_ConstructibleObjectMenu(a_object, a_count, a3, a4, a5);
	}

	void HUDHook::PlayPickupSoundAndMessage_AlchemyMenu(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5)
	{
		if (Settings::bEnableRecentLoot && !Settings::bRecentLootHideInCraftingMenus) {
			if (a_object->GetPlayable()) {
				HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count);
				if (Settings::bRecentLootHideVanillaMessage) {  // Skip original call
					if (a4) {                                   // Play pickup sound
						ConsoleRE::PlayerCharacter::GetSingleton()->PlayPickUpSound(a_object, a3, 0);
					}
					return;
				}
			}
		}

		_PlayPickupSoundAndMessage_AlchemyMenu(a_object, a_count, a3, a4, a5);
	}

	void HUDHook::PlayPickupSoundAndMessage_EnchantConstructMenu(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5)
	{
		if (Settings::bEnableRecentLoot && !Settings::bRecentLootHideInCraftingMenus) {
			if (a_object->GetPlayable()) {
				HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count);
				if (Settings::bRecentLootHideVanillaMessage) {  // Skip original call
					if (a4) {                                   // Play pickup sound
						ConsoleRE::PlayerCharacter::GetSingleton()->PlayPickUpSound(a_object, a3, 0);
					}
					return;
				}
			}
		}

		_PlayPickupSoundAndMessage_EnchantConstructMenu(a_object, a_count, a3, a4, a5);
	}

	void HUDHook::PlayPickupSoundAndMessage_AddItem(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5)
	{
		if (Settings::bEnableRecentLoot) {
			if (a_object->GetPlayable()) {
				//HUDHandler::GetSingleton()->AddRecentLootMessage(a_object, a_object->GetName(), a_count); // no need as it already triggers the hook placed at PlayerCharacter::AddObjectToContainer
				if (Settings::bRecentLootHideVanillaMessage) {  // Skip original call
					if (a4) {                                   // Play pickup sound
						ConsoleRE::PlayerCharacter::GetSingleton()->PlayPickUpSound(a_object, a3, 0);
					}
					return;
				}
			}
		}

		_PlayPickupSoundAndMessage_AddItem(a_object, a_count, a3, a4, a5);
	}

	bool HUDHook::IsPlayerOrPlayerMount(ConsoleRE::Actor* a_actor)
	{
		if (Settings::bPlayerWidgetDisplayMountStamina) 
		{
			ConsoleRE::ActorPtr playerMount = nullptr;
			bool bPlayerHasMount = ConsoleRE::PlayerCharacter::GetSingleton()->GetMount(playerMount);
			return a_actor->IsPlayerRef() || (bPlayerHasMount && a_actor == playerMount.get());
		} 
		else 
		{
			return a_actor->IsPlayerRef();
		}
	}

	void HUDHook::FlashHUDStaminaMountHook()
	{
		static REL::Relocation<uintptr_t> hook(0x6DEF80); // { RELOCATION_ID(36994, 38022) };  // 60E820, 636870

		struct Patch : Xbyak::CodeGenerator
		{
			explicit Patch(uintptr_t a_funcAddr) 
			{
				Xbyak::Label funcLabel;

				mov(rdi, r15);
				call(ptr[rip + funcLabel]);
				cmp(rax, true);

				// og
				lea(rbx, ptr[r15 + 0x100]);

				//
				jmp(ptr[rip]);
				dq(hook.address() + 0x1DE);

				L(funcLabel);
				dq(a_funcAddr);
			}
		};

		Patch patch(reinterpret_cast<uintptr_t>(IsPlayerOrPlayerMount));
		patch.ready();

		std::array<uint8_t, 11> data;
		memset(data.data(), 0x90, data.size());
		xUtilty::safe_write(hook.address() + 0x1D3, data.data(), 11);

		// REL::safe_fill(hook.address() + 0x1D3 /* 0x13B */, 0x90, 11);
		auto& trampoline = API::GetTrampoline();
		trampoline.write_branch<6>(hook.address() + 0x1D3 /* 0x13B */, trampoline.allocate(patch));
	}

	void DamageHook::HandleHealthDamage_Actor(ConsoleRE::Actor* a_this, ConsoleRE::Actor* a_attacker, float a_damage)
	{
		_HandleHealthDamage_Actor(a_this, a_attacker, a_damage);

		if (Settings::bEnableFloatingText && Settings::bEnableFloatingCombatText) {
			if (a_attacker == ConsoleRE::PlayerCharacter::GetSingleton()) {
				HUDHandler::GetSingleton()->AddFloatingWorldTextWidget(std::to_string(static_cast<int32_t>(ceil(-a_damage))), 0xFFFFFF, 2.f, false, a_this->GetLookingAtLocation());
			}
		}
	}

	void DamageHook::HandleHealthDamage_Character(ConsoleRE::Character* a_this, ConsoleRE::Actor* a_attacker, float a_damage)
	{
		_HandleHealthDamage_Character(a_this, a_attacker, a_damage);

		if (Settings::bEnableFloatingText && Settings::bEnableFloatingCombatText) {
			if (a_attacker == ConsoleRE::PlayerCharacter::GetSingleton()) {
				auto lastHitData = a_this->currentProcess->middleHigh->lastHitData;
				if (lastHitData && lastHitData->totalDamage == -a_damage) {  // physical attack
					HUDHandler::GetSingleton()->AddFloatingWorldTextWidget(std::to_string(static_cast<int32_t>(-ceil(a_damage))), 0xFFFFFF, 2.f, true, lastHitData->hitPosition);
				} else {  // spell/other
					HUDHandler::GetSingleton()->AddStackingDamageWorldTextWidget(a_this->GetHandle(), -a_damage);
					//HUDHandler::GetSingleton()->AddFloatingWorldTextWidget(std::to_string(static_cast<int32_t>(-ceil(a_damage))), 0xFFFFFF, 2.f, false, a_this->GetLookingAtLocation());
				}
			}
		}
	}

	void DamageHook::HandleHealthDamage_PlayerCharacter(ConsoleRE::PlayerCharacter* a_this, ConsoleRE::Actor* a_attacker, float a_damage)
	{
		_HandleHealthDamage_PlayerCharacter(a_this, a_attacker, a_damage);
		
		if (Settings::bEnableFloatingText && Settings::bEnableFloatingCombatText) {
			HUDHandler::GetSingleton()->AddFloatingWorldTextWidget(std::to_string(static_cast<int32_t>(ceil(-a_damage))), 0xFF0000, 2.f, false, a_this->GetLookingAtLocation());
		}
	}
}
