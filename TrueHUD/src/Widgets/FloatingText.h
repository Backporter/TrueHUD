#pragma once
#include "TrueHUDAPI.h"

namespace Scaleform
{
	class FloatingText : public TRUEHUD_API::WidgetBase
	{
	public:
		FloatingText(ConsoleRE::GPtr<ConsoleRE::GFxMovieView> a_view, uint32_t a_widgetID, std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint3 a_worldPosition) :
			WidgetBase(a_view, a_widgetID),
			_text(a_text),
			_color(a_color),
			_duration(a_duration),
			_bSpecial(a_bSpecial),
			_worldPosition(a_worldPosition),
			_bIs3D(true)
		{}

		FloatingText(ConsoleRE::GPtr<ConsoleRE::GFxMovieView> a_view, uint32_t a_widgetID, std::string a_text, uint32_t a_color, float a_duration, bool a_bSpecial, ConsoleRE::NiPoint2 a_screenPosition) :
			WidgetBase(a_view, a_widgetID),
			_text(a_text),
			_color(a_color),
			_duration(a_duration),
			_bSpecial(a_bSpecial),
			_screenPosition(a_screenPosition),
			_bIs3D(false)
		{}

		virtual void Update(float a_deltaTime) override;
		virtual void Initialize() override;
		virtual void Dispose() override;

	protected:
		virtual void UpdatePosition(float a_deltaTime);
		virtual void LoadConfig();

		std::string _text;
		uint32_t _color;
		float _duration;
		bool _bSpecial;
		ConsoleRE::NiPoint3 _worldPosition;
		ConsoleRE::NiPoint2 _screenPosition;
		bool _bIs3D;

		ConsoleRE::NiPoint3 _currentVelocity;
		ConsoleRE::NiPoint3 _gravityAcceleration{ 0.f, 0.f, -120.f };
	};
}
