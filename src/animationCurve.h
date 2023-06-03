#pragma once

#include "fbxnode.h"
#include "fbxobject.h"

namespace fbx
{
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// AnimationCurve

	struct AnimationCurve : FBXObject
	{
	protected:
		AnimationCurve();

	public:

		static const Type s_type = Type::ANIMATION_CURVE;
		static AnimationCurve* Create();
		static const char* GetClassName() { return "AnimationCurve"; }

		virtual int getKeyCount() const = 0;
		virtual const i64* getKeyTime() const = 0;
		virtual const float* getKeyValue() const = 0;
		virtual const int* getKeyFlag() const = 0;

		virtual double Evaluate(const OFBTime& time) const = 0;

	};
}

