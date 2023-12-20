#pragma once

#define PI       3.1415926535897932f
#define ROOT_TWO 1.4142135623730950f

namespace Utils
{
	// get world coordinates of nodeName for actor
	bool GetNodePosition(ConsoleRE::ActorPtr a_actor, const char* a_nodeName, ConsoleRE::NiPoint3& point);

	// acquire actor's torso position
	bool GetTorsoPos(ConsoleRE::ActorPtr a_actor, ConsoleRE::NiPoint3& point);

	bool GetTargetPos(ConsoleRE::ObjectRefHandle a_target, ConsoleRE::NiPoint3& pos, bool bGetTorsoPos = true);

	// returns true if actor is player's teammate, summon, or teammate's summon
	bool IsPlayerTeammateOrSummon(ConsoleRE::Actor* a_actor);

	bool IsBehindPlayerCamera(const ConsoleRE::NiPoint3& a_pos);

	inline bool FloatCompare(const float a, const float b)
	{
		double delta = fabs(a - b);
		if (delta < std::numeric_limits<float>::epsilon() &&
			delta > -std::numeric_limits<float>::epsilon()) {
			return true;
		}
		return false;
	}

	inline float GetPct(const float a_current, const float a_max)
	{
		float percent = -1.f;

		if (a_max < 0.f) {
			return percent;
		}

		if (!FloatCompare(a_max, 0.f)) {
			percent = a_current / a_max;
			return fmin(1.f, fmax(percent, -1.f));  // negative indicates that the actor value is not used
		}

		return percent;
	}

	inline float InterpEaseIn(const float& A, const float& B, float alpha, float exp)
	{
		float const modifiedAlpha = std::pow(alpha, exp);
		return std::lerp(A, B, modifiedAlpha);
	}

	inline float InterpEaseOut(const float& A, const float& B, float alpha, float exp)
	{
		float const modifiedAlpha = 1.f - pow(1.f - alpha, exp);
		return std::lerp(A, B, modifiedAlpha);
	}

	inline float InterpEaseInOut(const float& A, const float& B, float alpha, float exp)
	{
		return std::lerp(A, B, (alpha < 0.5f) ? InterpEaseIn(0.f, 1.f, alpha * 2.f, exp) * 0.5f : InterpEaseOut(0.f, 1.f, alpha * 2.f - 1.f, exp) * 0.5f + 0.5f);
	}

	inline ConsoleRE::NiPoint3 RotateAngleAxis(const ConsoleRE::NiPoint3& vec, const float angle, const ConsoleRE::NiPoint3& axis)
	{
		float S = sin(angle);
		float C = cos(angle);

		const float XX = axis.x * axis.x;
		const float YY = axis.y * axis.y;
		const float ZZ = axis.z * axis.z;

		const float XY = axis.x * axis.y;
		const float YZ = axis.y * axis.z;
		const float ZX = axis.z * axis.x;

		const float XS = axis.x * S;
		const float YS = axis.y * S;
		const float ZS = axis.z * S;

		const float OMC = 1.f - C;

		return ConsoleRE::NiPoint3(
			(OMC * XX + C) * vec.x + (OMC * XY - ZS) * vec.y + (OMC * ZX + YS) * vec.z,
			(OMC * XY + ZS) * vec.x + (OMC * YY + C) * vec.y + (OMC * YZ - XS) * vec.z,
			(OMC * ZX - YS) * vec.x + (OMC * YZ + XS) * vec.y + (OMC * ZZ + C) * vec.z);
	}

	inline ConsoleRE::NiPoint3 RotateVector(const ConsoleRE::NiPoint3& a_vec, const ConsoleRE::NiQuaternion& a_quat)
	{
		//http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
		const ConsoleRE::NiPoint3 Q{ a_quat.x, a_quat.y, a_quat.z };
		const ConsoleRE::NiPoint3 T = Q.Cross(a_vec) * 2.f;
		return a_vec + (T * a_quat.w) + Q.Cross(T);
	}

	inline ConsoleRE::NiPoint3 GetForwardVector(const ConsoleRE::NiQuaternion& a_quat) {
		return RotateVector({ 0.f, 1.f, 0.f }, a_quat);
	}

	inline bool IsNormalized(const ConsoleRE::NiPoint3& a_vector) {
		return fabs(1.f - a_vector.SqrLength()) < 0.01f;
	}

	inline void FindBestAxisVectors(const ConsoleRE::NiPoint3& a_vector, ConsoleRE::NiPoint3& a_outAxis1, ConsoleRE::NiPoint3& a_outAxis2) {
		float x = fabs(a_vector.x);
		float y = fabs(a_vector.y);
		float z = fabs(a_vector.z);

		if (z > x && z > y) {
			a_outAxis1 = { 1.f, 0.f, 0.f };
		} else {
			a_outAxis1 = { 0.f, 0.f, 1.f };
		}

		a_outAxis1 = a_outAxis1 - a_vector * (a_outAxis1.Dot(a_vector));
		a_outAxis1.Unitize();
		a_outAxis2 = a_outAxis1.Cross(a_vector);
	}

	ConsoleRE::NiQuaternion QuatFromRotationMatrix(const ConsoleRE::NiMatrix3& a_matrix);

	class Matrix4
	{
	public:
		enum class Axis : uint8_t
		{
			kX,
			kY,
			kZ
		};

		Matrix4() noexcept = default;
		Matrix4(const ConsoleRE::NiPoint3& a_x, const ConsoleRE::NiPoint3& a_y, const ConsoleRE::NiPoint3& a_z, const ConsoleRE::NiPoint3& a_w);

		void SetAxis(const ConsoleRE::NiPoint3& a_axis0, const ConsoleRE::NiPoint3& a_axis1, const ConsoleRE::NiPoint3& a_axis2, const ConsoleRE::NiPoint3& a_origin);
		ConsoleRE::NiPoint3 TransformVector4(const ConsoleRE::NiPoint3& a_vector, float a_w) const;
		ConsoleRE::NiPoint3 TransformPosition(const ConsoleRE::NiPoint3& a_position) const;
		ConsoleRE::NiPoint3 TransformVector(const ConsoleRE::NiPoint3& a_vector) const;
		
		inline ConsoleRE::NiPoint3 GetOrigin() const
		{
			return { entry[3][0], entry[3][1], entry[3][2] };
		}

		inline ConsoleRE::NiPoint3 GetScaledAxis(Axis a_axis)
		{
			switch (a_axis) {
			case Axis::kX:
				return { entry[0][0], entry[0][1], entry[0][2] };
			case Axis::kY:
				return { entry[1][0], entry[1][1], entry[1][2] };
			case Axis::kZ:
				return { entry[2][0], entry[2][1], entry[2][2] };
			default:
				return { 0.f, 0.f, 0.f };
			}
		}

		Matrix4 operator*(const Matrix4& a_other) const;

		// members
		float entry[4][4];
	};

	class MatrixScale : public Matrix4
	{
	public:
		MatrixScale(float a_scale);
		MatrixScale(const ConsoleRE::NiPoint3& a_scale);
	};

	class MatrixQuatRotation : public Matrix4
	{
	public:
		MatrixQuatRotation(const ConsoleRE::NiQuaternion& a_quat);
	};

	ConsoleRE::NiMatrix3 QuaternionToMatrix(const ConsoleRE::NiQuaternion& a_quat);
	[[nodiscard]] inline ConsoleRE::NiPoint3 TransformVectorByMatrix(const ConsoleRE::NiPoint3& a_vector, const ConsoleRE::NiMatrix3& a_matrix)
	{
		return ConsoleRE::NiPoint3(a_matrix.entry[0][0] * a_vector.x + a_matrix.entry[0][1] * a_vector.y + a_matrix.entry[0][2] * a_vector.z,
			a_matrix.entry[1][0] * a_vector.x + a_matrix.entry[1][1] * a_vector.y + a_matrix.entry[1][2] * a_vector.z,
			a_matrix.entry[2][0] * a_vector.x + a_matrix.entry[2][1] * a_vector.y + a_matrix.entry[2][2] * a_vector.z);
	}
}

namespace Scaleform
{
	inline void RegisterString(ConsoleRE::GFxValue* dst, const char* name, const char* str)
	{
		ConsoleRE::GFxValue fxValue;
		fxValue.SetString(str);
		dst->SetMember(name, fxValue);
	}

	inline void RegisterNumber(ConsoleRE::GFxValue* dst, const char* name, double value)
	{
		ConsoleRE::GFxValue fxValue;
		fxValue.SetNumber(value);
		dst->SetMember(name, fxValue);
	}

	inline void RegisterBoolean(ConsoleRE::GFxValue* dst, const char* name, bool value)
	{
		ConsoleRE::GFxValue fxValue;
		fxValue.SetBoolean(value);
		dst->SetMember(name, fxValue);
	}

}
