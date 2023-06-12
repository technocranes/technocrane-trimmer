
#include "model.h"
#include "animationCurveNode.h"
#include "nodeAttribute.h"

using namespace fbx;


Model::Model(int64_t id)
	: FBXObject(id)
{
}

int Model::GetAnimationNodeCount() const
{
	return static_cast<int>(m_AnimationNodes.size());
}
AnimationCurveNode* Model::GetAnimationNode(int index) const
{
	if (index < 0 || index >= GetAnimationNodeCount())
	{
		printf("model animation node is not found under the given index %d\n", index);
		return nullptr;
	}

	return m_AnimationNodes[index];
}

AnimationCurveNode* Model::FindAnimationNodeByName(const char* name) const
{
	for (auto node : m_AnimationNodes)
	{
		if (strcmp(node->GetName(), name) == 0)
		{
			return node;
		}
	}
	return nullptr;
}

void Model::OnDataConnectionNotify(fbx::ConnectionEvent connectionEvent, FBXObject* connectionObject, const Connection* connection)
{
	if (connectionEvent == fbx::ConnectionEvent::ADD_CHILD)
	{
		if (connectionObject->GetType() == FBXObject::Type::NODE_ATTRIBUTE)
		{
			m_NodeAttribute = reinterpret_cast<NodeAttribute*>(connectionObject);
		}
		else if (connectionObject->GetType() == FBXObject::Type::ANIMATION_CURVE_NODE)
		{
			m_AnimationNodes.push_back(reinterpret_cast<AnimationCurveNode*>(connectionObject));
		}
	}
}

