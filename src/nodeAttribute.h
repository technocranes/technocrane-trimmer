#pragma once

#include "fbxdocument.h"
#include "fbxnode.h"
#include "model.h"

namespace fbx
{
	struct NodeAttribute : public FBXObject
	{
	protected:
		NodeAttribute(int64_t id);

	public:

		static const Type s_type = Type::NODE_ATTRIBUTE;
		static NodeAttribute* Create(int64_t id);
		static const char* GetClassName() { return "NodeAttribute"; }

		virtual int GetAnimationNodeCount() const = 0;
		virtual AnimationCurveNode* GetAnimationNode(int index) const = 0;

		virtual AnimationCurveNode* FindAnimationNodeByName(const char* name) const = 0;

	};
}