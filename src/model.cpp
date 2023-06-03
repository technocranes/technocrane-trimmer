
#include "model.h"

using namespace fbx;


Model::Model()
	: FBXObject()
{
}

const int Model::GetAnimationNodeCount() const
{
	return static_cast<int>(m_AnimationNodes.size());
}
const AnimationCurveNode* Model::GetAnimationNode(int index) const
{
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

