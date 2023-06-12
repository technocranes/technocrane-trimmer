
#include "nodeAttribute.h"
#include "fbxdocument.h"
#include "model.h"
#include "animationCurveNode.h"

using namespace fbx;

struct NodeAttributeImpl : NodeAttribute
{
	NodeAttributeImpl(int64_t id)
		: NodeAttribute(id)
	{
	}


	void OnDataConnectionNotify(fbx::ConnectionEvent connnectionEvent, FBXObject* connectionObject, const Connection* connection) override
	{
		if (connnectionEvent == fbx::ConnectionEvent::PARENTED)
		{
			Model* curveNode = reinterpret_cast<Model*>(connectionObject);
			m_Owner = curveNode;
		}
		else if (connnectionEvent == fbx::ConnectionEvent::ADD_CHILD)
		{
			// do we have animation node to be added ?
			if (connectionObject->GetType() == FBXObject::Type::ANIMATION_CURVE_NODE)
			{
				m_AnimationNodes.push_back(reinterpret_cast<AnimationCurveNode*>(connectionObject));
			}
		}
	}


	Type GetType() const override { return Type::NODE_ATTRIBUTE; }


	void OnRetreive(const FBXDocument& _document, const FBXNode& _element) override
	{}

	void OnStore(FBXDocument& document, FBXNode& element) override
	{}

	int GetAnimationNodeCount() const override { return static_cast<int>(m_AnimationNodes.size()); }
	AnimationCurveNode* GetAnimationNode(int index) const override { return m_AnimationNodes[index]; }

	AnimationCurveNode* FindAnimationNodeByName(const char* name) const override
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

private:
	Model*									m_Owner{ nullptr };
	std::vector<AnimationCurveNode*>		m_AnimationNodes;
};




NodeAttribute::NodeAttribute(int64_t id)
	: FBXObject(id)
{}


NodeAttribute* NodeAttribute::Create(int64_t id)
{
	NodeAttributeImpl* curve = new NodeAttributeImpl(id);
	return curve;
}

