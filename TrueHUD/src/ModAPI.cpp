#include "ModAPI.h"
#include "HUDHandler.h"
#include "Settings.h"

namespace Messaging
{
	TrueHUDInterface::TrueHUDInterface() noexcept
	{
		apiTID = scePthreadGetthreadid();
	}

	TrueHUDInterface::~TrueHUDInterface() noexcept {}

	unsigned long TrueHUDInterface::GetTrueHUDThreadId() const noexcept
	{
		return apiTID;
	}

	APIResult TrueHUDInterface::RequestTargetControl(size_t a_modHandle) noexcept
	{
		const auto owner = targetControlOwner.load(std::memory_order::memory_order_acquire);
		if (owner != static_cast<size_t>(-1))
			if (owner == a_modHandle)
				return APIResult::AlreadyGiven;
			else
				return APIResult::AlreadyTaken;

		auto expected = static_cast<size_t>(static_cast<size_t>(-1));
		if (!targetControlOwner.compare_exchange_strong(expected, a_modHandle, std::memory_order::memory_order_acq_rel))
			return APIResult::AlreadyTaken;

		return APIResult::OK;
	}

	APIResult TrueHUDInterface::RequestSpecialResourceBarsControl(size_t a_modHandle) noexcept
	{
		const auto owner = specialResourceBarsControlOwner.load(std::memory_order::memory_order_acquire);
		if (owner != static_cast<size_t>(-1))
			if (owner == a_modHandle)
				return APIResult::AlreadyGiven;
			else
				return APIResult::AlreadyTaken;

		auto expected = static_cast<size_t>(static_cast<size_t>(-1));
		if (!specialResourceBarsControlOwner.compare_exchange_strong(expected, a_modHandle, std::memory_order::memory_order_acq_rel))
			return APIResult::AlreadyTaken;

		if (Settings::glob_trueHUDSpecialResourceBars) {
			Settings::glob_trueHUDSpecialResourceBars->value = true;
		}
		return APIResult::OK;
	}

	APIResult TrueHUDInterface::SetTarget(size_t a_modHandle, ConsoleRE::ActorHandle a_actorHandle) noexcept
	{
		if (targetControlOwner != a_modHandle) {
			return APIResult::NotOwner;
		}

		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->SetTarget(a_actorHandle);
		}

		return APIResult::OK;
	}

	APIResult TrueHUDInterface::SetSoftTarget(size_t a_modHandle, ConsoleRE::ActorHandle a_actorHandle) noexcept
	{
		if (targetControlOwner != a_modHandle) {
			return APIResult::NotOwner;
		}

		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->SetSoftTarget(a_actorHandle);
		}

		return APIResult::OK;
	}

	void TrueHUDInterface::AddActorInfoBar(ConsoleRE::ActorHandle a_actorHandle) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->AddActorInfoBar(a_actorHandle);
		}
	}

	void TrueHUDInterface::RemoveActorInfoBar(ConsoleRE::ActorHandle a_actorHandle, WidgetRemovalMode a_removalMode) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->RemoveActorInfoBar(a_actorHandle, static_cast<TRUEHUD_API::WidgetRemovalMode>(a_removalMode));
		}
	}

	void TrueHUDInterface::AddBoss(ConsoleRE::ActorHandle a_actorHandle) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->AddBossInfoBar(a_actorHandle);
		}
	}

	void TrueHUDInterface::RemoveBoss(ConsoleRE::ActorHandle a_actorHandle, WidgetRemovalMode a_removalMode) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->RemoveBossInfoBar(a_actorHandle, static_cast<TRUEHUD_API::WidgetRemovalMode>(a_removalMode));
		}
	}

	void TrueHUDInterface::FlashActorValue(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, bool a_bLong) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->FlashActorValue(a_actorHandle, a_actorValue, a_bLong);
		}
	}

	APIResult TrueHUDInterface::FlashActorSpecialBar(size_t a_modHandle, ConsoleRE::ActorHandle a_actorHandle, bool a_bLong) noexcept
	{
		if (specialResourceBarsControlOwner != a_modHandle) {
			return APIResult::NotOwner;
		}

		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->FlashActorSpecialBar(a_actorHandle, a_bLong);
		}

		return APIResult::OK;
	}

	APIResult TrueHUDInterface::RegisterSpecialResourceFunctions(size_t a_modHandle, SpecialResourceCallback&& a_getCurrentSpecialResource, SpecialResourceCallback&& a_getMaxSpecialResource, bool a_bSpecialMode, bool a_bDisplaySpecialForPlayer) noexcept
	{
		if (specialResourceBarsControlOwner != a_modHandle) {
			return APIResult::NotOwner;
		}

		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->GetCurrentSpecial = a_getCurrentSpecialResource;
			hudHandler->GetMaxSpecial = a_getMaxSpecialResource;
			hudHandler->bSpecialMode = a_bSpecialMode;
			hudHandler->bSpecialFunctionsProvided = true;
			hudHandler->bDisplaySpecialForPlayer = a_bDisplaySpecialForPlayer;
		}

		return APIResult::OK;
	}

	void TrueHUDInterface::LoadCustomWidgets(size_t a_modHandle, const char* a_filePath, APIResultCallback&& a_successCallback) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->LoadCustomWidgets(a_modHandle, a_filePath, std::forward<APIResultCallback>(a_successCallback));
		}
	}

	void TrueHUDInterface::RegisterNewWidgetType(size_t a_modHandle, uint32_t a_widgetType) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->RegisterNewWidgetType(a_modHandle, a_widgetType);
		}
	}

	void TrueHUDInterface::AddWidget(size_t a_modHandle, uint32_t a_widgetType, uint32_t a_widgetID, const char* a_symbolIdentifier, std::shared_ptr<WidgetBase> a_widget) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->AddWidget(a_modHandle, a_widgetType, a_widgetID, a_symbolIdentifier, a_widget);
		}
	}

	void TrueHUDInterface::RemoveWidget(size_t a_modHandle, uint32_t a_widgetType, uint32_t a_widgetID, WidgetRemovalMode a_removalMode) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->RemoveWidget(a_modHandle, a_widgetType, a_widgetID, a_removalMode);
		}
	}

	size_t TrueHUDInterface::GetTargetControlOwner() const noexcept
	{
		return targetControlOwner;
	}

	size_t TrueHUDInterface::GetPlayerWidgetBarColorsControlOwner() const noexcept
	{
		return static_cast<size_t>(-1);
	}

	size_t TrueHUDInterface::GetSpecialResourceBarControlOwner() const noexcept
	{
		return specialResourceBarsControlOwner;
	}

	APIResult TrueHUDInterface::ReleaseTargetControl(size_t a_modHandle) noexcept
	{
		if (targetControlOwner != a_modHandle)
			return APIResult::NotOwner;
		targetControlOwner.store(static_cast<size_t>(-1), std::memory_order::memory_order_release);

		return APIResult::OK;
	}

	APIResult TrueHUDInterface::ReleaseSpecialResourceBarControl(size_t a_modHandle) noexcept
	{
		if (specialResourceBarsControlOwner != a_modHandle)
			return APIResult::NotOwner;
		specialResourceBarsControlOwner.store(static_cast<size_t>(-1), std::memory_order::memory_order_release);

		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->GetCurrentSpecial = nullptr;
			hudHandler->GetMaxSpecial = nullptr;
			hudHandler->bSpecialFunctionsProvided = false;
		}

		if (Settings::glob_trueHUDSpecialResourceBars) {
			Settings::glob_trueHUDSpecialResourceBars->value = false;
		}

		return APIResult::OK;
	}

	void TrueHUDInterface::OverrideBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType, uint32_t a_color) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->OverrideBarColor(a_actorHandle, a_actorValue, a_colorType, a_color);
		}
	}

	void TrueHUDInterface::OverrideSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType, uint32_t a_color) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->OverrideSpecialBarColor(a_actorHandle, a_colorType, a_color);
		}
	}

	void TrueHUDInterface::RevertBarColor(ConsoleRE::ActorHandle a_actorHandle, ConsoleRE::ActorValue a_actorValue, BarColorType a_colorType) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->RevertBarColor(a_actorHandle, a_actorValue, a_colorType);
		}
	}

	void TrueHUDInterface::RevertSpecialBarColor(ConsoleRE::ActorHandle a_actorHandle, BarColorType a_colorType) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		if (hudHandler) {
			hudHandler->RevertSpecialBarColor(a_actorHandle, a_colorType);
		}
	}

	void TrueHUDInterface::DrawLine(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawLine(a_start, a_end, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawPoint(const ConsoleRE::NiPoint3& a_position, float a_size, float a_duration, uint32_t a_color) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawPoint(a_position, a_size, a_duration, a_color);
	}

	void TrueHUDInterface::DrawArrow(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_size, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawArrow(a_start, a_end, a_size, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawBox(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_extent, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawBox(a_center, a_extent, a_rotation, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawCircle(a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawHalfCircle(const ConsoleRE::NiPoint3& a_center, const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawHalfCircle(a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawSphere(const ConsoleRE::NiPoint3& a_origin, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawSphere(a_origin, a_radius, a_segments, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawCylinder(const ConsoleRE::NiPoint3& a_start, const ConsoleRE::NiPoint3& a_end, float a_radius, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawCylinder(a_start, a_end, a_radius, a_segments, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawCone(const ConsoleRE::NiPoint3& a_origin, const ConsoleRE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, uint32_t a_segments, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawCone(a_origin, a_direction, a_length, a_angleWidth, a_angleHeight, a_segments, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawCapsule(const ConsoleRE::NiPoint3& a_origin, float a_halfHeight, float a_radius, const ConsoleRE::NiQuaternion& a_rotation, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawCapsule(a_origin, a_halfHeight, a_radius, a_rotation, a_duration, a_color, a_thickness);
	}

	void TrueHUDInterface::DrawCapsule(const ConsoleRE::NiPoint3& a_vertexA, const ConsoleRE::NiPoint3& a_vertexB, float a_radius, float a_duration, uint32_t a_color, float a_thickness) noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();
		hudHandler->DrawCapsule(a_vertexA, a_vertexB, a_radius, a_duration, a_color, a_thickness);
	}

	bool TrueHUDInterface::HasInfoBar(ConsoleRE::ActorHandle a_actorHandle, bool a_bFloatingOnly /*= false*/) const noexcept
	{
		auto hudHandler = HUDHandler::GetSingleton();

		if (a_bFloatingOnly) {
			return hudHandler->HasActorInfoBar(a_actorHandle);
		} else {
			return hudHandler->HasActorInfoBar(a_actorHandle) || hudHandler->HasBossInfoBar(a_actorHandle);
		}
	}

	bool TrueHUDInterface::IsTargetControlTaken() const noexcept
	{
		return targetControlOwner.load(std::memory_order::memory_order_acquire) != static_cast<size_t>(-1);
	}

	bool TrueHUDInterface::IsSpecialResourceBarsControlTaken() const noexcept
	{
		return specialResourceBarsControlOwner.load(std::memory_order::memory_order_acquire) != static_cast<size_t>(-1);
	}
}
