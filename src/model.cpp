
#include "model.h"

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

void Model::OnDataConnectionNotify(fbx::ConnectionEvent connectionEvent, FBXObject* connectionObject, const Connection* connection)
{
	if (connectionEvent == fbx::ConnectionEvent::ADD_CHILD)
	{
		if (connectionObject->GetType() == FBXObject::Type::ANIMATION_CURVE_NODE)
		{
			m_AnimationNodes.push_back(reinterpret_cast<AnimationCurveNode*>(connectionObject));
		}
	}
}

