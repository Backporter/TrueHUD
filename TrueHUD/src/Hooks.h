#pragma once

namespace Hooks
{	
	class HUDHook
	{
	public:
		static void Hook()
		{
			//
			REL::Relocation<uintptr_t> EnemyHealthVtbl{ ConsoleRE::VTABLE_EnemyHealth[0] };
			_ProcessMessage = EnemyHealthVtbl.write_vfunc(0x3, ProcessMessage);

			//
			REL::Relocation<uintptr_t> SprintHandlerVtbl{ ConsoleRE::VTABLE_SprintHandler[0] };
			_ProcessButton = SprintHandlerVtbl.write_vfunc(0x3, ProcessButton);

			//
			REL::Relocation<uintptr_t> PlayerCharacterVtbl{ ConsoleRE::VTABLE_PlayerCharacter[0] };
			_AddObjectToContainer = PlayerCharacterVtbl.write_vfunc(0x5A, AddObjectToContainer);

			//
			_PickUpObject = PlayerCharacterVtbl.write_vfunc(0xD5, PickUpObject);

			auto& trampoline = API::GetTrampoline();

			REL::Relocation<uintptr_t> hook1(0xA85E40);						  // { RELOCATION_ID(51907, 52845) };  // 8D53D0, 905A10 // FlashHudMenuMeter
			REL::Relocation<uintptr_t> hook2(0xA0F0B0);						  // { RELOCATION_ID(50747, 51642) };  // 87FFF0, 8B0220 // SetHUDCartMode
			REL::Relocation<uintptr_t> hook3(0xA0F630);						  // { RELOCATION_ID(50771, 51666) };  // 881CC0, 8B1F40 // HUDChargeMeter::Update
			REL::Relocation<uintptr_t> hook4(0xB6D2B0);						  // { RELOCATION_ID(55946, 56490) };  // 9A42B0, 9CC5B0 // called by AddItemFunctor
			REL::Relocation<uintptr_t> hook5(0xF09A0);						  // { RELOCATION_ID(14692, 14864) };  // 19CA00, 1A7DE0 // called by ActivateFlora
			REL::Relocation<uintptr_t> hook6(0x9F1E20);						  // { RELOCATION_ID(50476, 51369) };  // 86E2C0, 89C750 // called by ConstructibleObjectMenu
			REL::Relocation<uintptr_t> hook7(0x9FE440);						  // { RELOCATION_ID(50449, 51354) };  // 86B980, 899C10 // called by AlchemyMenu
			REL::Relocation<uintptr_t> hook8(0x9FA0C0);						  // { RELOCATION_ID(50450, 51355) };  // 86C640, 89A9C0 // called by EnchantConstructMenu
			REL::Relocation<uintptr_t> hook9(0x14A9A0);						  // { RELOCATION_ID(15887, 16127) };  // 1E9900, 1F5130 // called by AddItem

			//
			_AddMessage_Flash			= trampoline.write_call<5>(hook1.address() + 0x9F,	AddMessage_Flash);                // 8D5453, 905A93
			_AddMessage_SetHUDCartMode	= trampoline.write_call<5>(hook2.address() + 0xC5,	AddMessage_SetHUDCartMode);		  // 880093, 8B02C3
			_Invoke_ChargeMeter1		= trampoline.write_call<5>(hook3.address() + 0x16B, Invoke_ChargeMeter1);             // 881E28, 8B20A8
			_Invoke_ChargeMeter2		= trampoline.write_call<5>(hook3.address() + 0x2D1, Invoke_ChargeMeter2);             // 881F43, 8B21F3

			//
			_AddItem_AddItemFunctor								= trampoline.write_call<5>(hook4.address() + 0x783, AddItem_AddItemFunctor);																	// 9A440D, 9CC70D
			_PlayPickupSoundAndMessage_AddItemFunctor			= trampoline.write_call<5>(hook4.address() + 0x81E /* RELOCATION_OFFSET(0x1C6, 0x1C6) */, PlayPickupSoundAndMessage_AddItemFunctor);			// 9A4476, 9CC776
			_PlayPickupSoundAndMessage_ActivateFlora1			= trampoline.write_call<5>(hook5.address() + 0x2D0 /* RELOCATION_OFFSET(0x250, 0x260) */, PlayPickupSoundAndMessage_ActivateFlora1);			// 19CC50, 1A8040
			_PlayPickupSoundAndMessage_ActivateFlora2			= trampoline.write_call<5>(hook5.address() + 0x463 /* RELOCATION_OFFSET(0x3C1, 0x3CD) */, PlayPickupSoundAndMessage_ActivateFlora2);			// 19CDC1, 1A81AD
			_PlayPickupSoundAndMessage_ConstructibleObjectMenu	= trampoline.write_call<5>(hook6.address() + 0x6F /* RELOCATION_OFFSET(0x5D, 0x68) */,	  PlayPickupSoundAndMessage_ConstructibleObjectMenu);	// 86E31D, 89C7B8
			_PlayPickupSoundAndMessage_AlchemyMenu				= trampoline.write_call<5>(hook7.address() + 0x2CD /* RELOCATION_OFFSET(0x22E, 0x229) */, PlayPickupSoundAndMessage_AlchemyMenu);               // 86BBAE, 899E39
			_PlayPickupSoundAndMessage_EnchantConstructMenu		= trampoline.write_call<5>(hook8.address() + 0x30B /* RELOCATION_OFFSET(0x2A5, 0x2A3) */, PlayPickupSoundAndMessage_EnchantConstructMenu);		// 86C8E5, 89AC63
			_PlayPickupSoundAndMessage_AddItem					= trampoline.write_call<5>(hook9.address() + 0x171 /* RELOCATION_OFFSET(0x178, 0x182) */, PlayPickupSoundAndMessage_AddItem);					// 1E9A78, 1F52B2
			
			//
			FlashHUDStaminaMountHook();
		}

	private:
		static bool ProcessMessage(uintptr_t a_enemyHealth, ConsoleRE::UIMessage* a_uiMessage);
		static inline REL::Relocation<decltype(ProcessMessage)> _ProcessMessage;

		static void ProcessButton(ConsoleRE::SprintHandler* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data);
		static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;

		static void AddObjectToContainer(ConsoleRE::Actor* a_this, ConsoleRE::TESBoundObject* a_object, ConsoleRE::ExtraDataList* a_extraList, int32_t a_count, ConsoleRE::TESObjectREFR* a_fromRefr);
		static inline REL::Relocation<decltype(AddObjectToContainer)> _AddObjectToContainer;

		static void PickUpObject(ConsoleRE::Actor* a_this, ConsoleRE::TESObjectREFR* a_object, uint32_t a_count, bool a_arg3, bool a_playSound);
		static inline REL::Relocation<decltype(PickUpObject)> _PickUpObject;

		static void AddMessage_Flash(ConsoleRE::UIMessageQueue* a_this, const ConsoleRE::BSFixedString& a_menuName, ConsoleRE::UI_MESSAGE_TYPE a_type, ConsoleRE::HUDData* a_data);
		static void AddMessage_SetHUDCartMode(ConsoleRE::UIMessageQueue* a_this, const ConsoleRE::BSFixedString& a_menuName, ConsoleRE::UI_MESSAGE_TYPE a_type, ConsoleRE::HUDData* a_data);
		static bool Invoke_ChargeMeter1(ConsoleRE::GFxValue::ObjectInterface* a_this, void* a_data, ConsoleRE::GFxValue* a_result, const char* a_name, const ConsoleRE::GFxValue* a_args, size_t a_numArgs, bool a_isDObj);
		static bool Invoke_ChargeMeter2(ConsoleRE::GFxValue::ObjectInterface* a_this, void* a_data, ConsoleRE::GFxValue* a_result, const char* a_name, const ConsoleRE::GFxValue* a_args, size_t a_numArgs, bool a_isDObj);

		static void AddItem_AddItemFunctor(ConsoleRE::TESObjectREFR* a_this, ConsoleRE::TESObjectREFR* a_object, int32_t a_count, bool a4, bool a5);
		static void PlayPickupSoundAndMessage_AddItemFunctor(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5);
		static void PlayPickupSoundAndMessage_ActivateFlora1(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5);
		static void PlayPickupSoundAndMessage_ActivateFlora2(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5);
		static void PlayPickupSoundAndMessage_ConstructibleObjectMenu(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5);
		static void PlayPickupSoundAndMessage_AlchemyMenu(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5);
		static void PlayPickupSoundAndMessage_EnchantConstructMenu(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5);
		static void PlayPickupSoundAndMessage_AddItem(ConsoleRE::TESBoundObject* a_object, int32_t a_count, bool a3, bool a4, void* a5);

		static bool IsPlayerOrPlayerMount(ConsoleRE::Actor* a_actor);
		static void FlashHUDStaminaMountHook();

		static inline REL::Relocation<decltype(AddMessage_Flash)> _AddMessage_Flash;
		static inline REL::Relocation<decltype(AddMessage_SetHUDCartMode)> _AddMessage_SetHUDCartMode;
		static inline REL::Relocation<decltype(Invoke_ChargeMeter1)> _Invoke_ChargeMeter1;
		static inline REL::Relocation<decltype(Invoke_ChargeMeter2)> _Invoke_ChargeMeter2;

		static inline REL::Relocation<decltype(AddItem_AddItemFunctor)> _AddItem_AddItemFunctor;
		static inline REL::Relocation<decltype(PlayPickupSoundAndMessage_AddItemFunctor)> _PlayPickupSoundAndMessage_AddItemFunctor;
		static inline REL::Relocation<decltype(PlayPickupSoundAndMessage_ActivateFlora1)> _PlayPickupSoundAndMessage_ActivateFlora1;
		static inline REL::Relocation<decltype(PlayPickupSoundAndMessage_ActivateFlora2)> _PlayPickupSoundAndMessage_ActivateFlora2;
		static inline REL::Relocation<decltype(PlayPickupSoundAndMessage_ConstructibleObjectMenu)> _PlayPickupSoundAndMessage_ConstructibleObjectMenu;
		static inline REL::Relocation<decltype(PlayPickupSoundAndMessage_AlchemyMenu)> _PlayPickupSoundAndMessage_AlchemyMenu;
		static inline REL::Relocation<decltype(PlayPickupSoundAndMessage_EnchantConstructMenu)> _PlayPickupSoundAndMessage_EnchantConstructMenu;
		static inline REL::Relocation<decltype(PlayPickupSoundAndMessage_AddItem)> _PlayPickupSoundAndMessage_AddItem;
	};

	class DamageHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> ActorVtbl{ ConsoleRE::VTABLE_Actor[0] };
			_HandleHealthDamage_Actor = ActorVtbl.write_vfunc(0x10E, HandleHealthDamage_Actor);

			REL::Relocation<uintptr_t> CharacterVtbl{ ConsoleRE::VTABLE_Character[0] };
			_HandleHealthDamage_Character = CharacterVtbl.write_vfunc(0x10E, HandleHealthDamage_Character);

			REL::Relocation<uintptr_t> PlayerCharacterVtbl{ ConsoleRE::VTABLE_PlayerCharacter[0] };
			_HandleHealthDamage_PlayerCharacter = PlayerCharacterVtbl.write_vfunc(0x10E, HandleHealthDamage_PlayerCharacter);
		}

	private:
		static void HandleHealthDamage_Actor(ConsoleRE::Actor* a_this, ConsoleRE::Actor* a_attacker, float a_damage);
		static inline REL::Relocation<decltype(HandleHealthDamage_Actor)> _HandleHealthDamage_Actor;

		static void HandleHealthDamage_Character(ConsoleRE::Character* a_this, ConsoleRE::Actor* a_attacker, float a_damage);
		static inline REL::Relocation<decltype(HandleHealthDamage_Character)> _HandleHealthDamage_Character;

		static void HandleHealthDamage_PlayerCharacter(ConsoleRE::PlayerCharacter* a_this, ConsoleRE::Actor* a_attacker, float a_damage);
		static inline REL::Relocation<decltype(HandleHealthDamage_PlayerCharacter)> _HandleHealthDamage_PlayerCharacter;
	};

	void Install();
}
