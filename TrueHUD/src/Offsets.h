#pragma once

// variables

static ConsoleRE::NiRect<float>* g_viewPort		= (ConsoleRE::NiRect<float>*)REL::Relocation<uintptr_t>(0x319B028).address();   // 2F4DED0, 2FE8B98

static float* g_fHUDOpacity						= (float*)REL::Relocation<uintptr_t>(0x31A03F8).address();						// 1DF58F8, 1E89D38
static float* g_fShoutMeterEndDuration			= (float*)REL::Relocation<uintptr_t>(0x3199A98).address();						// 1DF3ED8, 1E88290

static float* g_deltaTime						= (float*)REL::Relocation<uintptr_t>(0x31C7478).address();						// 2F6B948, 30064C8
static float* g_deltaTimeRealTime				= (float*)REL::Relocation<uintptr_t>(0x31C747C).address();						// 2F6B94C, 30064CC
static float* g_DurationOfApplicationRunTimeMS	= (float*)REL::Relocation<uintptr_t>(0x31C7480).address();						// 2F6B950, 30064D0

static uintptr_t g_worldToCamMatrix				= REL::Relocation<uintptr_t>(0x319B050).address();								// 2F4C910, 2FE75F0

//
static float* g_fNear							= (float*)(REL::Relocation<uintptr_t>(0x313E748) /* RELOCATION_ID(517032, 403540) */.address() + 0x40);             // 2F26FC0, 2FC1A90
static float* g_fFar							= (float*)(REL::Relocation<uintptr_t>(0x313E748) /* RELOCATION_ID(517032, 403540) */.address() + 0x44);             // 2F26FC4, 2FC1A94


// functions
typedef uint32_t(*tIsSentient)(ConsoleRE::Actor* a_this);
typedef uint32_t(*tGetSoulType)(uint16_t a_actorLevel, uint8_t a_isActorSentient);
typedef void(*tSetHUDMode)(const char* a_mode, bool a_enable);
typedef void(*tFlashHUDMenuMeter)(ConsoleRE::ActorValue a_actorValue);


static REL::Relocation<tIsSentient>			IsSentient		  (0x7227C0); // { RELOCATION_ID(36889, 37913) };
static REL::Relocation<tGetSoulType>		GetSoulType		  (0x43F770); // { RELOCATION_ID(25933, 26520) };
static REL::Relocation<tSetHUDMode>			SetHUDMode		  (0xA0F0B0); // { RELOCATION_ID(50747, 51642) };
static REL::Relocation<tFlashHUDMenuMeter>	FlashHUDMenuMeter (0xA85E40); // { RELOCATION_ID(51907, 52845) };
