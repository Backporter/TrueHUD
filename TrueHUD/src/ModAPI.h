#pragma once
#include "TrueHUDAPI.h"

class HUDHandler;

namespace Messaging
{
	using APIResult = ::TRUEHUD_API::APIResult;
	using InterfaceVersion1 = ::TRUEHUD_API::IVTrueHUD1;
	using InterfaceVersion2 = ::TRUEHUD_API::IVTrueHUD2;
	using InterfaceVersion3 = ::TRUEHUD_API::IVTrueHUD3;
	using InterfaceVersion4 = ::TRUEHUD_API::IVTrueHUD4;
	using PlayerWidgetBarType = ::TRUEHUD_API::PlayerWidgetBarType;
	using BarColorType = ::TRUEHUD_API::BarColorType;
	using WidgetRemovalMode = ::TRUEHUD_API::WidgetRemovalMode;

	using SpecialResourceCallback = ::TRUEHUD_API::SpecialResourceCallback;
	using APIResultCallback = ::TRUEHUD_API::APIResultCallback;

	using WidgetBase = ::TRUEHUD_API::WidgetBase;

	class TrueHUDInterface : public InterfaceVersion4
	{
	private:
		TrueHUDInterface() noexcept;
		virtual ~TrueHUDInterface() noexcept;

	public:
		static TrueHUDInterface* GetSingleton() noexcept
		{
			static TrueHUDInterface singleton;
			return std::addressof(singleton);
		}

		// InterfaceVersion1
		virtual unsigned long GetTrueHUDThreadId() const noexcept override;
		virtual APIResult RequestTargetControl(size_t a_modHandle) noexcept override;
		virtual APIResult RequestSpecialResourceBarsControl(size_t a_modHandle) noexcept override;
		virtual APIResult SetTarget(size_t a_modHandle, ConsoleRE::ActorHandle a_actorHandle) noexcept override;
		virtual APIResult SetSoftTarget(size_t a_modHandle, ConsoleRE::ActorHandle a_actorHandle) noexcept override;
		virtual void AddActorInfoBar(ConsoleRE::ActorHandle a_actorHandle) noexcept override;
		virtual void RemoveActorInfoBar(ConsoleRE::ActorHandle a_actorHandle, WidgetRemovalMode a_removalMode) noexcept override;
		virtual void AddBoss(ConsoleRE::ActorHandle a_actorHandle) noexcept override;
		virtual void RemoveBoss(ConsoleRE::ActorHandle a_actorHandle, WidgetRemovalMode a_removalMode) noexcept override;
		virtual void FlashActorValue(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, bool a_bLong) noexcept override;
		virtual APIResult FlashActorSpecialBar(size_t a_modHandle, ConsoleRE::ActorHandle a_actorHandle, bool a_bLong) noexcept override;
		virtual APIResult RegisterSpecialResourceFunctions(size_t a_modHandle, SpecialResourceCallback&& a_getCurrentSpecialResource, SpecialResourceCallback&& a_getMaxSpecialResource, bool a_bSpecialMode, bool a_bDisplaySpecialForPlayer) noexcept override;
		virtual void LoadCustomWidgets(size_t a_modHandle, const char* a_filePath, APIResultCallback&& a_successCallback) noexcept override;
		virtual void RegisterNewWidgetType(size_t a_modHandle, uint32_t a_widgetType) noexcept override;
		virtual void AddWidget(size_t a_modHandle, uint32_t a_widgetType, uint32_t a_widgetID, const char* a_symbolIdentifier, std::shared_ptr<WidgetBase> a_widget) noexcept override;
		virtual void RemoveWidget(size_t a_modHandle, uint32_t a_widgetType, uint32_t a_widgetID, WidgetRemovalMode a_removalMode) noexcept override;
		virtual size_t GetTargetControlOwner() const noexcept override;
		virtual size_t GetPlayerWidgetBarColorsControlOwner() const noexcept override;
		virtual size_t GetSpecialResourceBarControlOwner() const noexcept override;
		virtual APIResult ReleaseTargetControl(size_t a_modHandle) noexcept override;
		virtual APIResult ReleaseSpecialResourceBarControl(size_t a_modHandle) noexcept override;

		// InterfaceVersion2
		virtual void OverrideBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType, uint32_t a_color) noexcept override;
		virtual void OverrideSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType, uint32_t a_color) noexcept override;
		virtual void RevertBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType) noexcept override;
		virtual void RevertSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType) noexcept override;

		// InterfaceVersion3
		virtual void DrawLine(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawPoint(const ConsoleRE::NiPoint3& a_position, float a_size, float a_duration, uint32_t a_color) noexcept override;
		virtual void DrawArrow(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_size, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawBox(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_extent, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawHalfCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawSphere(const ConsoleRE::NiPoint3& a_origin, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawCylinder(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawCone(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept override;
		virtual void DrawCapsule(const ConsoleRE::NiPoint3& a_origin, float a_halfHeight, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness) noexcept override;

		virtual bool HasInfoBar(ConsoleRE::ActorHandle a_actorHandle, bool a_bFloatingOnly = false) const noexcept override;

		// InterfaceVersion4
		virtual void DrawCapsule(const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, float a_duration, uint32_t a_color, float a_thickness) noexcept override;

		// Does a mod have control over the current target?
		bool IsTargetControlTaken() const noexcept;
		// Does a mod have control over the special resource bars?
		bool IsSpecialResourceBarsControlTaken() const noexcept;

	private:
		unsigned long apiTID = 0;

		std::atomic<size_t> targetControlOwner{ static_cast<size_t>(-1) };
		std::atomic<size_t> specialResourceBarsControlOwner{ static_cast<size_t>(-1) };
	};

}
