#pragma once
#include <unordered_set>
#include <queue>

#include "TrueHUDAPI.h"
#include "Widgets/ActorInfoBar.h"
#include "Widgets/BossInfoBar.h"
#include "Scaleform/TrueHUDMenu.h"

class HUDHandler :
	public ConsoleRE::BSTEventSink<ConsoleRE::TESCombatEvent>,
	public ConsoleRE::BSTEventSink<ConsoleRE::TESDeathEvent>,
	public ConsoleRE::BSTEventSink<ConsoleRE::TESEnterBleedoutEvent>,
	public ConsoleRE::BSTEventSink<ConsoleRE::TESHitEvent>,
	public ConsoleRE::BSTEventSink<ConsoleRE::MenuOpenCloseEvent>
{
private:
	using EventResult = ConsoleRE::BSEventNotifyControl;
	using TrueHUDMenu = Scaleform::TrueHUDMenu;
	using BarColorType = ::TRUEHUD_API::BarColorType;
	using WidgetRemovalMode = TRUEHUD_API::WidgetRemovalMode;
	using SpecialResourceCallback = TRUEHUD_API::SpecialResourceCallback;
	using APIResultCallback = TRUEHUD_API::APIResultCallback;
	using WidgetBase = TRUEHUD_API::WidgetBase;
	using MenuVisibilityMode = TrueHUDMenu::MenuVisibilityMode;

public:
	static HUDHandler* GetSingleton()
	{
		static HUDHandler singleton;
		return std::addressof(singleton);
	}

	static void Register();

	virtual EventResult ProcessEvent(const ConsoleRE::TESCombatEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESCombatEvent>* a_eventSource) override;
	virtual EventResult ProcessEvent(const ConsoleRE::TESDeathEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESDeathEvent>* a_eventSource) override;
	virtual EventResult ProcessEvent(const ConsoleRE::TESEnterBleedoutEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESEnterBleedoutEvent>* a_eventSource) override;
	virtual EventResult ProcessEvent(const ConsoleRE::TESHitEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESHitEvent>* a_eventSource) override;
	virtual EventResult ProcessEvent(const ConsoleRE::MenuOpenCloseEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::MenuOpenCloseEvent>* a_eventSource) override;

	void OpenTrueHUDMenu();
	void CloseTrueHUDMenu();

	bool IsTrueHUDMenuOpen();
	static ConsoleRE::GPtr<TrueHUDMenu> GetTrueHUDMenu();

	void Update();

	ConsoleRE::ObjectRefHandle GetTarget() const;
	ConsoleRE::ObjectRefHandle GetSoftTarget() const;
	void SetTarget(ConsoleRE::ObjectRefHandle a_actorHandle);
	void SetSoftTarget(ConsoleRE::ObjectRefHandle a_actorHandle);	

	bool HasActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
	void AddActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
	void RemoveActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode = WidgetRemovalMode::Immediate);

	bool HasBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
	void AddBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
	void RemoveBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode = WidgetRemovalMode::Immediate);

	void AddShoutIndicator();
	void RemoveShoutIndicator();

	void AddPlayerWidget();
	void RemovePlayerWidget();
	void UpdatePlayerWidgetChargeMeters(float a_percent, bool a_bForce, bool a_bLeftHand, bool a_bShow);

	void AddRecentLootWidget();
	void RemoveRecentLootWidget();
	void AddRecentLootMessage(ConsoleRE::TESBoundObject* a_object, const char* a_name, uint32_t a_count);

	void AddFloatingWorldTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint3 a_worldPosition);
	void AddFloatingScreenTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint2 a_screenPosition);
	void AddStackingDamageWorldTextWidget(ConsoleRE::ObjectRefHandle a_refHandle, float a_damage);

	void OverrideBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType, uint32_t a_color);
	void OverrideSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType, uint32_t a_color);
	void RevertBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType);
	void RevertSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType);

	void LoadCustomWidgets(size_t a_pluginHandle, const char* a_filePath, APIResultCallback&& a_successCallback);
	void RegisterNewWidgetType(size_t a_pluginHandle, uint32_t a_widgetType);
	void AddWidget(size_t a_pluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, const char* a_symbolIdentifier, std::shared_ptr<WidgetBase> a_widget);
	void RemoveWidget(size_t a_pluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, WidgetRemovalMode a_removalMode);

	void FlashActorValue(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, bool a_bLong);
	void FlashActorSpecialBar(ConsoleRE::ObjectRefHandle a_actorHandle, bool a_bLong);

	void DrawLine(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_duration, uint32_t a_color, float a_thickness);
	void DrawPoint(const ConsoleRE::NiPoint3& a_position, float a_size, float a_duration, uint32_t a_color);
	void DrawArrow(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_size, float a_duration, uint32_t a_color, float a_thickness);
	void DrawBox(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_extent, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness);
	void DrawCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness);
	void DrawHalfCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness);
	void DrawSphere(const ConsoleRE::NiPoint3& a_origin, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness);
	void DrawCylinder(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness);
	void DrawCone(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness);
	void DrawCapsule(const ConsoleRE::NiPoint3& a_origin, float a_halfHeight, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness);
	void DrawCapsule(const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, float a_duration, uint32_t a_color, float a_thickness);

	void SetMenuVisibilityMode(MenuVisibilityMode a_mode);
	void SetCartMode(bool a_enable);

	void RemoveAllWidgets();

	bool CheckActorForBoss(ConsoleRE::ObjectRefHandle a_refHandle);

	void OnPreLoadGame();
	void OnSettingsUpdated();

	void Initialize();

	bool bSpecialFunctionsProvided = false;
	SpecialResourceCallback GetCurrentSpecial = nullptr;
	SpecialResourceCallback GetMaxSpecial = nullptr;
	bool bSpecialMode;
	bool bDisplaySpecialForPlayer;

public:
	// friend class TrueHUDMenu;

	void Process(TrueHUDMenu& a_menu, float a_deltaTime);

private:
	using HUDTask = std::function<void(TrueHUDMenu&)>;
	using Lock = std::recursive_mutex;
	using Locker = std::lock_guard<Lock>;

	HUDHandler();
	HUDHandler(const HUDHandler&) = delete;
	HUDHandler(HUDHandler&&) = delete;

	~HUDHandler() = default;

	HUDHandler& operator=(const HUDHandler&) = delete;
	HUDHandler& operator=(HUDHandler&&) = delete;

	void AddHUDTask(HUDTask a_task);

	mutable Lock _lock;
	std::queue<HUDTask> _taskQueue;

	ConsoleRE::ObjectRefHandle _currentTarget;
	ConsoleRE::ObjectRefHandle _currentSoftTarget;

	struct DamageStack
	{
		DamageStack(float a_damage) :
			damage(a_damage)
		{}

		float damage = 0.f;
		float timeElapsed = 0.f;
	};
	std::unordered_map<ConsoleRE::ObjectRefHandle, DamageStack> _stackingDamage;
	constexpr static float _stackingPeriodDuration = 0.5f;
};
