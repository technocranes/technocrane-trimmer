#pragma once

#include "fbxnode.h"
#include "fbxobject.h"

namespace fbx
{
	enum AnimationNodeType
	{
		ANIMATIONNODE_TYPE_CUSTOM,
		ANIMATIONNODE_TYPE_TRANSLATION,
		ANIMATIONNODE_TYPE_ROTATION,
		ANIMATIONNODE_TYPE_SCALING,
		ANIMATIONNODE_TYPE_VISIBILITY,
		ANIMATIONNODE_TYPE_FIELD_OF_VIEW
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// AnimationCurveNode

	struct AnimationCurveNode : FBXObject
	{
	protected:
		AnimationCurveNode();

	public:
		static const Type s_type = Type::ANIMATION_CURVE_NODE;
		static AnimationCurveNode* Create();
		static const char* GetClassName() { return "AnimationCurveNode"; }

		
		virtual OFBVector3 getNodeLocalTransform(double time) const = 0;
		virtual const FBXObject* GetOwner() const = 0;

		// return next anim node linked under property layers stack (in order how layers have been sorted)
		virtual AnimationCurveNode* GetNext() = 0;
		virtual const AnimationCurveNode* GetNext() const = 0;
		virtual void LinkNext(const AnimationCurveNode* pNext) = 0;

		virtual AnimationLayer* getLayer() const = 0;

		virtual int GetCurveCount() const = 0;
		virtual AnimationCurve* GetCurve(int index) = 0;
		virtual const AnimationCurve* GetCurve(int index) const = 0;

		virtual bool Evaluate(double* Data, const OFBTime pTime) const = 0;

	};
}

