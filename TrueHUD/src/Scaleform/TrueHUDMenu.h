#pragma once

#include "TrueHUDAPI.h"
#include "Widgets/ActorInfoBar.h"
#include "Widgets/BossInfoBar.h"
#include "Widgets/ShoutIndicator.h"
#include "Widgets/PlayerWidget.h"
#include "Widgets/FloatingText.h"
#include "Widgets/RecentLoot.h"
#include "Offsets.h"
#include <chrono>
#include <unordered_set>

namespace std
{
	template <class T>
	struct hash<ConsoleRE::BSPointerHandle<T>>
	{
		uint32_t operator()(const ConsoleRE::BSPointerHandle<T>& a_handle) const
		{
			uint32_t nativeHandle = const_cast<ConsoleRE::BSPointerHandle<T>*>(&a_handle)->native_handle();  // ugh
			return nativeHandle;
		}
	};
}

namespace Scaleform
{
	class DebugLine
	{
	public:
		DebugLine(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, uint32_t a_color, float a_thickness, float a_duration) :
			start(a_start),
			end(a_end),
			color(a_color),
			thickness(a_thickness),
			duration(a_duration)
		{}

		ConsoleRE::NiPoint3 start;
		ConsoleRE::NiPoint3 end;
		uint32_t color;
		float thickness;
		float duration;
	};

	class DebugPoint
	{
	public:
		DebugPoint(const ConsoleRE::NiPoint3& a_position, uint32_t a_color, float a_size, float a_duration) :
			position(a_position),
			color(a_color),
			size(a_size),
			duration(a_duration)
		{}

		ConsoleRE::NiPoint3 position;
		uint32_t color;
		float size;
		float duration;
	};

	class TrueHUDMenu : public ConsoleRE::IMenu
	{
	private:
		using Super = ConsoleRE::IMenu;
		using BarColorType = ::TRUEHUD_API::BarColorType;
		using WidgetRemovalMode = TRUEHUD_API::WidgetRemovalMode;
		using WidgetBase = TRUEHUD_API::WidgetBase;
		using APIResultCallback = TRUEHUD_API::APIResultCallback;

		enum class TrueHUDWidgetType : uint8_t
		{
			kInfoBar = 0,
			kBossBar = 1,
			kStandaloneShoutIndicator = 2,
			kPlayerWidget = 3,
			kRecentLootWidget = 4,
			kFloatingText = 5
		};

		enum class BarType : uint8_t
		{
			kHealth,
			kMagicka,
			kStamina,
			kSpecial
		};

		struct BarColorOverride
		{
			uint32_t barColor = 0xFFFFFF;
			uint32_t phantomColor = 0xFFFFFF;
			uint32_t backgroundColor = 0xFFFFFF;
			uint32_t penaltyColor = 0xFFFFFF;
			uint32_t flashColor = 0xFFFFFF;

			bool bOverrideBarColor = false;
			bool bOverridePhantomColor = false;
			bool bOverrideBackgroundColor = false;
			bool bOverridePenaltyColor = false;
			bool bOverrideFlashColor = false;

			bool IsEmpty() const;
			bool GetColor(BarColorType a_colorType, uint32_t& a_outColor) const;
			void SetOverride(BarColorType a_colorType, uint32_t a_color);
			void RemoveOverride(BarColorType a_colorType);
		};

	public:
		enum class MenuVisibilityMode : uint8_t
		{
			kHidden,
			kPartial,
			kVisible
		};

		static constexpr const char* MenuName() noexcept { return MENU_NAME; }
		static constexpr int8_t SortPriority() noexcept { return SORT_PRIORITY; }

		static void Register();

		bool IsOpen() const;

		ConsoleRE::GPtr<ConsoleRE::GFxMovieView> GetView() const;

		void SetTarget(ConsoleRE::ObjectRefHandle a_actorHandle);
		void SetSoftTarget(ConsoleRE::ObjectRefHandle a_actorHandle);
		ConsoleRE::ObjectRefHandle GetTarget() const;
		ConsoleRE::ObjectRefHandle GetSoftTarget() const;
		
		bool HasActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
		bool AddActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
		bool RemoveActorInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode);

		bool HasBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
		bool AddBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle);
		bool RemoveBossInfoBar(ConsoleRE::ObjectRefHandle a_actorHandle, WidgetRemovalMode a_removalMode);

		bool AddShoutIndicator();
		bool RemoveShoutIndicator();

		bool AddPlayerWidget();
		bool RemovePlayerWidget();
		void UpdatePlayerWidgetChargeMeters(float a_percent, bool a_bForce, bool a_bLeftHand, bool a_bShow);

		bool AddRecentLootWidget();
		bool RemoveRecentLootWidget();
		void AddRecentLootMessage(ConsoleRE::TESBoundObject* a_object, const char* a_name, uint32_t a_count);

		bool AddFloatingWorldTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint3 a_worldPosition);
		bool AddFloatingScreenTextWidget(std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint2 a_screenPosition);

		bool LoadCustomWidgets(size_t a_myPluginHandle, const char* a_filePath, APIResultCallback&& a_successCallback);
		bool RegisterNewWidgetType(size_t a_myPluginHandle, uint32_t a_widgetType);
		bool AddWidget(size_t a_myPluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, const char* a_symbolIdentifier, std::shared_ptr<WidgetBase> a_widget);
		bool RemoveWidget(size_t a_myPluginHandle, uint32_t a_widgetType, uint32_t a_widgetID, WidgetRemovalMode a_removalMode);

		void FlashActorValue(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, bool a_bLong);
		void FlashActorSpecialBar(ConsoleRE::ObjectRefHandle a_actorHandle, bool a_bLong);

		void OverrideBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType, uint32_t a_color);
		void OverrideSpecialBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, BarColorType a_colorType, uint32_t a_color);
		void RevertBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType);
		void RevertSpecialBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, BarColorType a_colorType);
		uint32_t GetBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType) const;
		uint32_t GetSpecialBarColor(ConsoleRE::ObjectRefHandle a_actorHandle, BarColorType a_colorType) const;
		uint32_t GetDefaultColor(BarType a_barType, BarColorType a_barColorType) const;

		void DrawLine(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawPoint(const ConsoleRE::NiPoint3& a_position, float a_size, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF);
		void DrawArrow(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_size = 10.f, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawBox(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_extent, const ConsoleRE::NiQuaternion& a_rotation, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawHalfCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawSphere(const ConsoleRE::NiPoint3& a_origin, float a_radius, uint32_t a_segments = 16, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawCylinder(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_radius, uint32_t a_segments, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawCone(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, uint32_t a_segments, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawCapsule(const ConsoleRE::NiPoint3& a_origin, float a_halfHeight, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawCapsule(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);
		void DrawCapsule(const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, float a_duration = 0.f, uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f);

		void SetMenuVisibilityMode(MenuVisibilityMode a_mode);
		void SetCartMode(bool a_enable);

		void UpdateSettings();
		void RemoveAllWidgets();

	protected:
		using UIResult = ConsoleRE::UI_MESSAGE_RESULTS;

		TrueHUDMenu()
		{
			auto menu = static_cast<Super*>(this);
			menu->depthPriority = SortPriority();
			auto scaleformManager = ConsoleRE::BSScaleformManager::GetSingleton();

			// do NOT use LoadMovieEx if you want skse scaleform hooks, it doesn't call the original function which is hooked by skse
			[[maybe_unused]] const auto success = scaleformManager->LoadMovieEx(menu, FILE_NAME, [](ConsoleRE::GFxMovieDef* a_def) -> void { a_def->SetState(ConsoleRE::GFxState::StateType::kLog, ConsoleRE::make_gptr<Logger>().get()); });

			// this works instead
			//[[maybe_unused]] const auto success = scaleformManager->LoadMovie(menu, menu->uiMovie, FILE_NAME.data());
			assert(success);
			if (menu && menu->uiMovie) 
			{
				auto def = menu->uiMovie->GetMovieDef();
				if (def) 
				{
					def->SetState(ConsoleRE::GFxState::StateType::kLog, ConsoleRE::make_gptr<Logger>().get());
				}
			}

			menuFlags |= (uint32_t)ConsoleRE::UI_MENU_FLAGS::kAlwaysOpen;
			menuFlags |= (uint32_t)ConsoleRE::UI_MENU_FLAGS::kRequiresUpdate;
			menuFlags |= (uint32_t)ConsoleRE::UI_MENU_FLAGS::kAllowSaving;
			//menuFlags.set(ConsoleRE::UI_MENU_FLAGS::kCustomRendering);
			
			menu->inputContext = Context::kNone;

			_view = menu->uiMovie;
			_view->SetMouseCursorCount(0);	// disable input
		}

		TrueHUDMenu(const TrueHUDMenu&) = default;
		TrueHUDMenu(TrueHUDMenu&&) = default;

		~TrueHUDMenu() = default;

		TrueHUDMenu& operator=(const TrueHUDMenu&) = default;
		TrueHUDMenu& operator=(TrueHUDMenu&&) = default;

		static ConsoleRE::IMenu* Creator() { return new TrueHUDMenu(); }

		// IMenu
		void PostCreate() {
			OnOpen();
			Super::PostCreate();
		}

		UIResult ProcessMessage(ConsoleRE::UIMessage& a_message) override {
			using Type = ConsoleRE::UI_MESSAGE_TYPE;

			switch (a_message.type) 
			{
			case Type::kShow:
				OnOpen();
				return Super::ProcessMessage(a_message);
			case Type::kHide:
				OnClose();
				return Super::ProcessMessage(a_message);
			default:
				return Super::ProcessMessage(a_message);
			}
		}

		void AdvanceMovie(float, uint32_t) override
		{
			auto currentTime = chrono_clock::now();
			auto dt = (currentTime - _movieLastTime) / 1000000000.f;

			_movieLastTime = currentTime;
			float deltaTime = static_cast<float>(dt.count());

			ProcessDelegate(deltaTime);
			Update(deltaTime);
			//Super::AdvanceMovie(a_interval, a_currentTime); ??

			if (uiMovie) {
				uiMovie->Advance(deltaTime);
			}
		}

	private:
		class Logger : public ConsoleRE::GFxLog
		{
		public:
			void LogMessageVarg(LogMessageType, const char* a_fmt, va_list a_argList) override
			{
				std::string fmt(a_fmt ? a_fmt : "");
				while (!fmt.empty() && fmt.back() == '\n') {
					fmt.pop_back();
				}

				va_list args;
				va_copy(args, a_argList);
				std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
				std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
				va_end(args);

				xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "%s: %s", TrueHUDMenu::MenuName(), buf.data());
			}
		};

		using Lock = std::recursive_mutex;
		using Locker = std::lock_guard<Lock>;
		using chrono_clock = std::chrono::steady_clock;

		mutable Lock _lock;

		void OnOpen();
		void OnClose();

		void ProcessDelegate(float a_deltaTime);
		void Update(float a_deltaTime);

		void CacheData();

		void UpdateColors();
		void UpdateVisibility();

		bool AddBossInfoBarWidget(ConsoleRE::ObjectRefHandle a_actorHandle);
		int32_t GetNextBossBarIndex() const;
		void RefreshBossBarIndexes(int32_t a_removedIndex);
		void UpdateBossQueue();
		void AddToDepthsArray(std::shared_ptr<TRUEHUD_API::WidgetBase> a_widget, uint32_t a_widgetType, ConsoleRE::GFxValue& a_array);

		void UpdateDebugDraw(float a_deltaTime);

		void DrawLine2D(ConsoleRE::NiPoint2& a_start, ConsoleRE::NiPoint2& a_end, uint32_t a_color, float a_thickness);
		void DrawPoint2D(ConsoleRE::NiPoint2& a_position, uint32_t a_color, float a_size);
		void DrawLine3D(ConsoleRE::NiPoint3& a_start, ConsoleRE::NiPoint3& a_end, uint32_t a_color, float a_thickness);
		void DrawPoint3D(ConsoleRE::NiPoint3& a_position, uint32_t a_color, float a_size);
		void ClearLines();

		ConsoleRE::NiPoint2 WorldToScreen(const ConsoleRE::NiPoint3& a_worldPos) const;

		void ClampToScreen(ConsoleRE::NiPoint2& a_point);

		bool IsOnScreen(ConsoleRE::NiPoint2& a_start, ConsoleRE::NiPoint2& a_end) const;
		bool IsOnScreen(ConsoleRE::NiPoint2& a_point) const;

		bool _bIsOpen = false;

		bool _bCachedData = false;
		ConsoleRE::NiPoint2 _screenRes;

		MenuVisibilityMode _menuVisibilityMode = MenuVisibilityMode::kVisible;
		bool _bCartMode = false;
		
		std::unordered_map<ConsoleRE::ObjectRefHandle, std::shared_ptr<ActorInfoBar>> _actorInfoBarMap;
		std::unordered_map<ConsoleRE::ObjectRefHandle, std::shared_ptr<BossInfoBar>> _bossInfoBarMap;
		std::shared_ptr<ShoutIndicator> _shoutIndicator;
		std::shared_ptr<PlayerWidget> _playerWidget;
		std::shared_ptr<RecentLoot> _recentLoot;
		std::unordered_map<uint32_t, std::shared_ptr<FloatingText>> _floatingTextMap;
		uint32_t _nextFloatingTextID = 0;

		using CustomWidgets = std::unordered_map<uint32_t, std::shared_ptr<WidgetBase>>;
		using CustomWidgetTypes = std::unordered_map<uint32_t, CustomWidgets>;
		using CustomWidgetMap = std::unordered_map<size_t, CustomWidgetTypes>;

		CustomWidgetMap _customWidgets;

		ConsoleRE::ObjectRefHandle _targetHandle;
		ConsoleRE::ObjectRefHandle _softTargetHandle;

		std::list<ConsoleRE::ObjectRefHandle> _bossQueue;

		std::unordered_map<ConsoleRE::ObjectRefHandle, std::unordered_map<BarType, BarColorOverride>> _colorOverrides;
		std::unordered_set<ConsoleRE::ObjectRefHandle> _pendingColorChanges;

		std::vector<std::unique_ptr<DebugLine>> _linesToDraw;
		std::vector<std::unique_ptr<DebugPoint>> _pointsToDraw;

		bool _bVanillaEnemyHealthAlphaSaved = false;
		bool _bVanillaEnemyHealthHidden = false;
		ConsoleRE::GFxValue _savedVanillaEnemyHealthAlpha;

		bool _bSubtitleYSaved = false;
		ConsoleRE::GFxValue _savedSubtitleY;

		bool _bCompassAlphaSaved = false;
		ConsoleRE::GFxValue _savedCompassAlpha;

		constexpr static const char* FILE_NAME{ "TrueHUD" };
		constexpr static const char* MENU_NAME{ "TrueHUD" };
		constexpr static int8_t SORT_PRIORITY{ 0 };

		ConsoleRE::GPtr<ConsoleRE::GFxMovieView> _view;

		chrono_clock::time_point _movieLastTime;
	};
}
